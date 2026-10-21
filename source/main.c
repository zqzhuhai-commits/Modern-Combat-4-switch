/* SPDX-License-Identifier: MIT. Standalone AArch32 Modern Combat 4 runtime.
 * Bring-up: verify original library -> load ELF -> resolve imports ->
 * EGL/GLES2 -> JNI bootstrap -> frame loop.
 */
#include "port_config.h"
#include "mc4.h"
#include "jni.h"
#include "imports.h"
#include "dcr_path.h"
#include "dcr_time.h"
#include "dcr_sched.h"
#include "rt_applet.h"
#include "rt_boot.h"
#include "rt_window.h"
#include "selfproc.h"
#include "error.h"
#include "util.h"
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <switch.h>

Mc4Api mc4_api;
static so_module module;
static int running;
static EGLDisplay display;
static EGLSurface surface;
static EGLContext context;

extern EGLDisplay b_eglGetDisplay(EGLNativeDisplayType);
extern EGLBoolean b_eglInitialize(EGLDisplay, EGLint *, EGLint *);
extern EGLBoolean b_eglChooseConfig(EGLDisplay, const EGLint *, EGLConfig *, EGLint, EGLint *);
extern EGLContext b_eglCreateContext(EGLDisplay, EGLConfig, EGLContext, const EGLint *);
extern EGLSurface b_eglCreateWindowSurface(EGLDisplay, EGLConfig, void *, const EGLint *);
extern EGLBoolean b_eglMakeCurrent(EGLDisplay, EGLSurface, EGLSurface, EGLContext);
extern EGLBoolean b_eglSwapBuffers(EGLDisplay, EGLSurface);
extern void dcr_audio_pause(int paused);
extern void dcr_audio_close(void);

void dcr_config_load(void);
extern void mc4_swp_region(uint32_t start, uint32_t end);
static void stage(const char *s) { debugPrintf("[mc4/stage] %s\n", s); mc4_jni_check(s); log_flush_ring(); }
static uintptr_t lookup(void *m, const char *s) { return so_try_find_addr_rx(m, s); }
static void note(const char *s) { stage(s); }
static void *make_string(const char *s) { return jni_str(s); }
static void *make_class(const char *s) { JClass *c = jni_class(s); return c ? c->obj : NULL; }

static int verified(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    Sha256Context c;
    sha256ContextCreate(&c);
    unsigned char b[16384], h[32];
    size_t n, total = 0;
    while ((n = fread(b, 1, sizeof b, f))) { sha256ContextUpdate(&c, b, n); total += n; }
    int error = ferror(f);
    fclose(f);
    sha256ContextGetHash(&c, h);
    char hex[65];
    for (int i = 0; i < 32; i++) snprintf(hex + 2 * i, 3, "%02x", h[i]);
    debugPrintf("[mc4/library] %u bytes SHA256 %s\n", (unsigned)total, hex);
    return !error && total == MC4_LIB_SIZE && !strcmp(hex, MC4_LIB_SHA256);
}

const char *port_path_fixup(const char *real, char *out, size_t cap) {
    /* OBB and data live under <root>/; keep the real path as-is. */
    return real;
}

static int required_data(void) {
    const char *names[] = {
        "obb/" "main.1100.com.gameloft.android.ANMP.GloftM4HM.obb",
        "main.1100.com.gameloft.android.ANMP.GloftM4HM.obb",
        "assets"
    };
    int ok = 1;
    char path[768];
    for (unsigned i = 0; i < 3; i++) {
        snprintf(path, sizeof path, "%s/%s", dcr_game_root(), names[i]);
        struct stat st;
        if (stat(path, &st)) { debugPrintf("[mc4/assets] missing %s\n", names[i]); ok = 0; }
        else debugPrintf("[mc4/assets] %s present (%lld bytes)\n", names[i], (long long)st.st_size);
    }
    return ok ? 0 : -1;
}

static int load_game(void) {
    stage("validate original data"); if (required_data()) return -1;
    char path[768];
    snprintf(path, sizeof path, "%s/libModernCombat4.so", dcr_game_root());
    if (!verified(path)) {
        snprintf(path, sizeof path, "%s/lib/armeabi-v7a/libModernCombat4.so", dcr_game_root());
        if (!verified(path)) { stage("unsupported or missing original library"); return -1; }
    }
    stage("load ELF");
    if (dcr_self_process() == INVALID_HANDLE) { stage("process handle unavailable"); return -1; }
    int rc = so_load(&module, path, NULL, PORT_SO_REGION_BYTES);
    if (rc) { debugPrintf("[mc4] so_load=%d\n", rc); return -1; }
    if (so_relocate(&module)) { stage("relocation failed"); return -1; }
    int missing = so_resolve(&module, dcr_imports, dcr_imports_count, 1);
    debugPrintf("[mc4] unresolved imports=%d\n", missing);
    if (missing) return -1;
    if (so_fix_kuser_helpers(&module) < 0) return -1;
    so_finalize(&module);
    so_flush_caches(&module);
    for (int i = 0; i < module.phnum; i++)
        if (module.phdr[i].p_type == PT_LOAD && (module.phdr[i].p_flags & PF_X)) {
            uint32_t begin = (uint32_t)(uintptr_t)module.load_virtbase + module.phdr[i].p_vaddr;
            mc4_swp_region(begin, begin + module.phdr[i].p_memsz);
            break;
        }
    int n = mc4_bind(&mc4_api, lookup, &module, note);
    if (n) { debugPrintf("[mc4] missing native exports=%d\n", n); return -1; }
    stage("ELF mapped, imports resolved, native exports bound");
    return 0;
}

static int graphics(void) {
    stage("create EGL/GLES2 window");
    dcr_window_set_size(MC4_OUTPUT_WIDTH, MC4_OUTPUT_HEIGHT);
    display = b_eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLint maj = 0, min = 0;
    if (!b_eglInitialize(display, &maj, &min)) return -1;
    EGLint attrs[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                      EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
                      EGL_DEPTH_SIZE, 24, EGL_STENCIL_SIZE, 8, EGL_NONE};
    EGLConfig config;
    EGLint count = 0;
    if (!b_eglChooseConfig(display, attrs, &config, 1, &count)) return -1;
    if (!count) {
        attrs[13] = 16; attrs[15] = 0;
        if (!b_eglChooseConfig(display, attrs, &config, 1, &count) || !count) return -1;
    }
    const EGLint ca[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    context = b_eglCreateContext(display, config, EGL_NO_CONTEXT, ca);
    surface = b_eglCreateWindowSurface(display, config, NULL, NULL);
    if (!context || !surface || !b_eglMakeCurrent(display, surface, surface, context)) return -1;
    debugPrintf("[mc4/gl] vendor=%s renderer=%s GLES=%s\n",
                glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION));
    eglSwapInterval(display, 1);
    glClearColor(0.015f, 0.025f, 0.04f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    if (!b_eglSwapBuffers(display, surface)) return -1;
    stage("EGL surface presented; calling original game next");
    return 0;
}

void port_focus_lost(void) {
    if (running) { mc4_input_release(); dcr_audio_pause(1); mc4_api.pause(g_jni_env, NULL); stage("native paused"); }
}
void port_focus_gained(void) {
    if (running) { mc4_api.resume(g_jni_env, NULL); dcr_audio_pause(0); stage("native resumed"); }
}

static int run_game(void) {
    stage("initialize JNI object model"); jni_init();
    extern int g_so_trace_ctors; g_so_trace_ctors = 1;
    stage("execute native constructors"); so_execute_init_array(&module); so_free_temp(&module);
    if (graphics()) { stage("EGL creation/presentation failed"); return -1; }
    debugPrintf("[mc4/display] output=%dx%d fullscreen; native asset coordinates=%dx%d\n",
                MC4_OUTPUT_WIDTH, MC4_OUTPUT_HEIGHT, MC4_WIDTH, MC4_HEIGHT);
    stage("JNI/native bootstrap begins");
    int rc = mc4_bootstrap(&mc4_api, g_jni_vm, g_jni_env, make_string, make_class,
                           MC4_WIDTH, MC4_HEIGHT, 0, note);
    debugPrintf("[mc4] bootstrap result=%d\n", rc);
    if (rc) return -1;
    stage("native bootstrap returned; entering frame loop");
    mc4_input_init();
    running = 1;
    unsigned frames = 0;
    while (appletMainLoop() && !rt_exit_requested()) {
        rt_applet_poll();
        if (!rt_focused()) { svcSleepThread(50000000); continue; }
        if (mc4_input_poll()) break;
        int result = mc4_api.step(g_jni_env, NULL);
        if (frames < 3) debugPrintf("[mc4/frame] step returned %d frame=%u\n", result, frames);
        if (!b_eglSwapBuffers(display, surface)) { stage("EGL swap failed"); break; }
        if (++frames % 300 == 0) {
            debugPrintf("[mc4/frame] presented=%u JNI_objects=%d\n", frames, jni_live_objects());
            log_flush_ring();
        }
    }
    stage("native shutdown");
    mc4_input_release();
    dcr_audio_close();
    running = 0;
    rt_applet_stop();
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(display, surface);
    eglDestroyContext(display, context);
    eglTerminate(display);
    return 0;
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    const char *root = dcr_game_root();
    mkdir(root, 0777);
    log_init(root);
    log_console_open();
    log_console_show_text();
    debugPrintf("Modern Combat 4 runtime 0.1.0 / BOOT TEST / gameplay unverified\n");
    dcr_report_boot();
    if (chdir(root)) debugPrintf("[mc4] root chdir failed\n");
    dcr_config_load();
    dcr_time_init();
    dcr_path_prepare_dirs();
    debugPrintf("A: attempt original game initialization. PLUS: exit.\n");
    log_flush_ring();
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);
    int start = 0;
    while (appletMainLoop()) {
        padUpdate(&pad);
        u64 down = padGetButtonsDown(&pad);
        if (down & HidNpadButton_Plus) break;
        if (down & HidNpadButton_A) { start = 1; break; }
        log_console_update();
        svcSleepThread(16000000);
    }
    if (!start) return 0;
    if (load_game() == 0) {
        dcr_sched_init();
        rt_applet_start();
        if (run_game() == 0) { log_flush_ring(); return 0; }
    }
    stage("boot stopped; see debug.log / crash.log");
    while (appletMainLoop()) {
        padUpdate(&pad);
        if (padGetButtonsDown(&pad) & HidNpadButton_Plus) break;
        if (!display) log_console_update();
        svcSleepThread(16000000);
    }
    log_flush_ring();
    return 1;
}

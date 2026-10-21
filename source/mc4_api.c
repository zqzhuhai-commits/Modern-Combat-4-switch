/* SPDX-License-Identifier: MIT
 * mc4_api.c -- bind Modern Combat 4 native exports and run the bootstrap
 * sequence of the Gameloft GL2JNI engine.
 */
#include "mc4.h"
#include <string.h>

#define P(prefix, name) prefix name

int mc4_bind(Mc4Api *api, mc4_lookup lookup, void *module, mc4_log log) {
    int missing = 0;
    if (!api || !lookup) return -1;
    memset(api, 0, sizeof(*api));
#define BIND(field, symbol) do { \
    uintptr_t addr = lookup(module, symbol); \
    _Static_assert(sizeof(api->field) == sizeof(addr), "unsupported pointer ABI"); \
    memcpy(&api->field, &addr, sizeof(addr)); \
    if (!addr) { ++missing; if (log) log(symbol); } \
} while (0)
    BIND(on_load, "JNI_OnLoad");
    /* GL2JNILib engine */
    BIND(init, GL2_PREFIX "init");
    BIND(init_gl, GL2_PREFIX "initGL");
    BIND(step, GL2_PREFIX "step");
    BIND(resize, GL2_PREFIX "resize");
    BIND(init_view_settings, GL2_PREFIX "InitViewSettings");
    BIND(set_paths, GL2_PREFIX "setPaths");
    BIND(state_changed, GL2_PREFIX "stateChanged");
    BIND(touch, GL2_PREFIX "touchEvent");
    BIND(key_down, GL2_PREFIX "OnKeyDown");
    BIND(key_up, GL2_PREFIX "OnKeyUp");
    BIND(keyboard, GL2_PREFIX "keyboardEvent");
    BIND(destroy, GL2_PREFIX "destroy");
    /* MC4 game */
    BIND(game_init, MC4_PREFIX "MC4_nativeMC4Init");
    BIND(pause, MC4_PREFIX "MC4_pauseGame");
    BIND(resume, MC4_PREFIX "MC4_resumeGame");
    BIND(splash, MC4_PREFIX "MC4_splashScreenFunc");
    BIND(handle_key, MC4_PREFIX "MC4_nativeHandleKey");
    /* auxiliary inits */
    BIND(device_init, MC4_PREFIX "GLUtils_Device_nativeInit");
    BIND(data_init, MC4_PREFIX "DataSharing_nativeInit");
    BIND(sutils_init, MC4_PREFIX "GLUtils_SUtils_nativeInit");
    BIND(iab_init, MC4_PREFIX "iab_InAppBilling_nativeInit");
    BIND(social_init, "Java_com_gameloft_GLSocialLib_PlatformAndroid_nativeInit");
    BIND(sendinfo_init, MC4_PREFIX "SendInfo_initMethods");
    BIND(videoview_init, MC4_PREFIX "MyVideoView_Init");
    /* installer / GDRM policy */
    BIND(installer_start, MC4_PREFIX "installer_GameInstaller_nativeStart");
    BIND(gdrm_allow, MC4_PREFIX "installer_GDRMPolicy_nativeAllow");
#undef BIND
    return missing;
}

int mc4_bootstrap(const Mc4Api *a, void *vm, void *env,
                  void *(*make_string)(const char *),
                  void *(*make_class)(const char *),
                  int width, int height, int language, mc4_log log) {
    if (!a || !vm || !env || !make_string || !make_class ||
        width <= 0 || height <= 0) return -1;
    if (!a->on_load || !a->init || !a->init_gl || !a->step ||
        !a->resize || !a->game_init) return -2;

    void *mc4_class = make_class(MC4_CLASS "MC4");
    void *gl2_class = make_class(GL2_CLASS);
    void *device_class = make_class(MC4_CLASS "GLUtils/Device");
    void *data_class = make_class(MC4_CLASS "DataSharing");
    void *sutils_class = make_class(MC4_CLASS "GLUtils/SUtils");
    if (!mc4_class || !gl2_class || !device_class || !data_class || !sutils_class) return -5;

    if (log) log("JNI_OnLoad");
    int version = a->on_load(vm, NULL);
    if (version != 0x00010001 && version != 0x00010002 &&
        version != 0x00010004 && version != 0x00010006) return -4;

    /* auxiliary inits, per Gameloft GL2 bootstrap */
    if (a->device_init)  { if (log) log("GLUtils.Device.nativeInit");  a->device_init(env, device_class); }
    if (a->data_init)    { if (log) log("DataSharing.nativeInit");     a->data_init(env, data_class); }
    if (a->sutils_init)  { if (log) log("GLUtils.SUtils.nativeInit");  a->sutils_init(env, sutils_class); }
    if (a->iab_init)     { if (log) log("IAB.nativeInit");             a->iab_init(env, make_class(MC4_CLASS "iab/InAppBilling")); }
    if (a->social_init)  { if (log) log("GLSocialLib.PlatformAndroid.nativeInit"); a->social_init(env, make_class("com/gameloft/GLSocialLib/PlatformAndroid")); }
    if (a->sendinfo_init){ if (log) log("SendInfo.initMethods");       a->sendinfo_init(env, make_class(MC4_CLASS "SendInfo")); }
    if (a->videoview_init){if (log) log("MyVideoView.Init");           a->videoview_init(env, make_class(MC4_CLASS "MyVideoView")); }

    /* engine init */
    if (log) log("GL2JNILib.InitViewSettings");
    a->init_view_settings(env, gl2_class);
    if (log) log("GL2JNILib.setPaths");
    void *p0 = make_string("/sdcard/");
    void *p1 = make_string("/sdcard/");
    void *p2 = make_string("main.1100.com.gameloft.android.ANMP.GloftM4HM.obb");
    void *p3 = make_string("patch.12222.com.gameloft.android.ANMP.GloftM4HM.obb");
    a->set_paths(env, gl2_class, p0, p1, p2, p3);
    if (log) log("GL2JNILib.init");
    a->init(env, gl2_class);
    if (log) log("GL2JNILib.initGL");
    a->init_gl(env, gl2_class);
    if (log) log("GL2JNILib.resize");
    a->resize(env, gl2_class, width, height);

    /* game init */
    if (log) log("MC4.nativeMC4Init");
    a->game_init(env, mc4_class);
    if (a->splash) { if (log) log("MC4.splashScreenFunc"); a->splash(env, mc4_class, 0); }
    if (a->state_changed) { if (log) log("GL2JNILib.stateChanged"); a->state_changed(env, gl2_class, 0); }
    return 0;
}

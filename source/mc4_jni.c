/* SPDX-License-Identifier: MIT. MC4-specific JNI; reference:
 * Shadow-Guardian-vita and the supplied native ELF's JNI exports.
 * Exact class names are from libModernCombat4.so; unknown calls stay logged.
 */
#include "mc4.h"
#include "jni.h"
#include "rt_applet.h"
#include "dcr_path.h"
extern void dcr_audio_pause(int paused); /* opensles.c */
#include "util.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

const JMethodDef jni_method_defs[] = {{NULL, NULL, NULL, NULL}};
const JFieldDef jni_field_defs[] = {
    {"android/content/Context", "TELEPHONY_SERVICE", NULL, 0, "phone"},
    {"android/os/Build", "MODEL", NULL, 0, "Nintendo Switch"},
    {"android/os/Build", "MANUFACTURER", NULL, 0, "Nintendo"},
    {"android/os/Build$VERSION", "SDK_INT", NULL, 10, NULL},
    {NULL, NULL, NULL, 0, NULL}
};
const char *const jni_class_supers[][2] = {
    {MC4_CLASS "MC4", "android/app/Activity"},
    {GL2_CLASS, "java/lang/Object"},
    {"android/app/Activity", "android/content/Context"},
    {"android/content/Context", "java/lang/Object"},
    {NULL, NULL}
};
const char *const jni_missing_classes[] = {NULL};

static unsigned unknown, reported;
static int is_mc4(const char *c) { return !strncmp(c, MC4_CLASS, strlen(MC4_CLASS)); }
static int is_gl2(const char *c) { return !strcmp(c, GL2_CLASS); }

static JObj *byte_string(const char *s) {
    size_t n = strlen(s) + 1;
    JObj *o = jni_array('B', (int)n);
    if (o) memcpy(o->a.data, s, n);
    return o;
}

int port_jni_invoke(JObj *self, JMethod *m, const jvalue *a, jvalue *out) {
    const char *c = m->cls->name, *n = m->name;
    if ((!strcmp(c, "java/lang/System") &&
         (!strcmp(n, "currentTimeMillis") || !strcmp(n, "nanoTime"))) ||
        (is_mc4(c) && !strcmp(n, "GetCurrentTime"))) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        *out = !strcmp(n, "nanoTime")
                   ? jv_j((int64_t)ts.tv_sec * 1000000000 + ts.tv_nsec)
                   : jv_j((int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
        return 1;
    }
    if (!strcmp(c, "android/media/AudioTrack")) {
        if (!strcmp(n, "<init>")) { *out = jv_none(); return 1; }
        if (!strcmp(n, "getMinBufferSize")) { *out = jv_i(16384); return 1; }
        if (!strcmp(n, "play")) { dcr_audio_pause(0); *out = jv_none(); return 1; }
        if (!strcmp(n, "pause") || !strcmp(n, "stop") || !strcmp(n, "release")) { dcr_audio_pause(1); *out = jv_none(); return 1; }
    }
    /* Device identifiers: the Switch has none; return empty strings. */
    if (!strcmp(c, "android/telephony/TelephonyManager")) {
        if (!strcmp(n, "getDeviceId") || !strcmp(n, "getSubscriberId") ||
            !strcmp(n, "getLine1Number") || !strcmp(n, "getNetworkOperatorName") ||
            !strcmp(n, "getSimOperatorName")) { *out = jv_l(jni_str("")); return 1; }
        if (!strcmp(n, "getNetworkCountryIso") || !strcmp(n, "getSimCountryIso")) { *out = jv_l(jni_str("us")); return 1; }
        if (!strcmp(n, "getSimState")) { *out = jv_i(1); return 1; } /* ABSENT */
        if (!strcmp(n, "isNetworkRoaming")) { *out = jv_z(0); return 1; }
    }
    /* GLUtils.Device / SUtils obfuscated byte-array accessors: empty. */
    if ((is_mc4(c) || !strcmp(c, "android/os/Build")) && !strcmp(m->sig, "()[B")) {
        if (!strcmp(n, "a") || !strcmp(n, "b") || !strcmp(n, "c") || !strcmp(n, "d") ||
            !strcmp(n, "getHostName") || !strcmp(n, "getImei") || !strcmp(n, "getIMEI")) {
            *out = jv_l(byte_string("")); return 1;
        }
    }
    if (is_gl2(c) || is_mc4(c) || !strcmp(c, "android/content/Context") ||
        !strcmp(c, "android/app/Activity") || !strcmp(c, "java/io/File")) {
        /* handled below */
    } else if (!strncmp(c, "com/gameloft/", 13)) {
        /* GDRMPolicy / GameInstaller / IAB / Social stubs return permissive
         * values; the game's own objects are trusted. */
        if (!strcmp(n, "nativeAllow") || !strcmp(n, "initNativeAP") ||
            !strcmp(n, "processServer") || !strcmp(n, "setConst") ||
            !strcmp(n, "setLongConst") || !strcmp(n, "setTime") ||
            !strcmp(n, "getConstString") || !strcmp(n, "getPublicKey") ||
            !strcmp(n, "nativeGetDevID") || !strcmp(n, "nativeSendData") ||
            !strcmp(n, "nativeSetContext") || !strcmp(n, "nativeSetIABObject") ||
            !strcmp(n, "nativeInit") || !strcmp(n, "initMethods") ||
            !strcmp(n, "Init") || !strcmp(n, "onVideoFinished") ||
            !strcmp(n, "SetFinished")) {
            *out = jv_none(); return 1;
        }
        if (!strcmp(n, "nativeStart") || !strcmp(n, "nativeOnFBDataLoad") ||
            !strcmp(n, "nativeOnFBDialogDidComplete") ||
            !strcmp(n, "nativeOnFBDialogDidNotComplete") ||
            !strcmp(n, "nativeOnFBFailWithError") ||
            !strcmp(n, "nativeGameAPIComplete") || !strcmp(n, "nativeGameAPICompleteWithData") ||
            !strcmp(n, "nativeGameAPIDidNotComplete") || !strcmp(n, "nativeGameAPIDidPlusOneButton") ||
            !strcmp(n, "nativeGameAPINotifyLogout") || !strcmp(n, "nativeGameAPISetCanceled") ||
            !strcmp(n, "nativeOnKakaoDataError") || !strcmp(n, "nativeOnKakaoDataLoad") ||
            !strcmp(n, "nativeOnRRDataLoad") || !strcmp(n, "nativeOnRRDialogDidComplete") ||
            !strcmp(n, "nativeOnRRDialogDidNotComplete") || !strcmp(n, "nativeOnRRFailWithError") ||
            !strcmp(n, "nativeOnVKDataLoad") || !strcmp(n, "nativeOnVKDialogDidComplete") ||
            !strcmp(n, "nativeOnVKDialogDidNotComplete") || !strcmp(n, "nativeOnVKFailWithError") ||
            !strcmp(n, "nativeOnSWDataLoad") || !strcmp(n, "nativeOnSWDialogDidComplete") ||
            !strcmp(n, "nativeOnSWDialogDidNotComplete") || !strcmp(n, "nativeOnSWFailWithError")) {
            *out = jv_none(); return 1;
        }
    }
    /* PlayMovie: the Switch port skips the FMV (see mc4.cfg skip_intro_video). */
    if (!strncmp(n, "playMovie", 9) || !strcmp(n, "playVideo")) {
        debugPrintf("[mc4] skipping video %s\n", n);
        *out = jv_none(); return 1;
    }
    if (!strcmp(n, "Exit") || !strcmp(n, "sendAppToBackground") || !strcmp(n, "finish")) {
        debugPrintf("[mc4] original game requested exit\n");
        log_flush_ring();
        exit(0);
    }
    if (!strcmp(n, "getPath") || !strcmp(n, "getAbsolutePath")) {
        *out = jv_l(jni_str("/sdcard/gameloft/games/GloftM4HM/")); return 1;
    }
    if (!strcmp(n, "getSystemService") && strstr(m->sig, "Ljava/lang/String;")) {
        *out = jv_l(!strcmp(jni_utf(a[0].l), "phone")
                        ? jni_singleton("android/telephony/TelephonyManager")
                        : NULL);
        return 1;
    }
    if (!strcmp(n, "getFilesDir")) { *out = jv_l(jni_singleton("java/io/File")); return 1; }
    if (!strcmp(n, "getPackageName")) { *out = jv_l(jni_str(PORT_PACKAGE)); return 1; }
    if (!strcmp(n, "getContext") || !strcmp(n, "getInstance") || !strcmp(n, "getApplicationContext")) {
        *out = jv_l(jni_singleton(MC4_CLASS "MC4")); return 1;
    }
    if (!strcmp(n, "getResources")) { *out = jv_l(jni_singleton("android/content/res/Resources")); return 1; }
    if (!strcmp(n, "getAssets")) { *out = jv_l(jni_singleton("android/content/res/AssetManager")); return 1; }
    if (!strcmp(n, "getCountry") || !strcmp(n, "getNetworkCountryIso") || !strcmp(n, "getSimCountryIso")) {
        *out = jv_l(jni_str("US")); return 1;
    }
    if (!strcmp(n, "getLanguage") || !strcmp(n, "GetPhoneLanguage") || !strcmp(n, "getDeviceLanguage")) {
        *out = jv_l(jni_str("zh")); return 1;
    }
    if (!strcmp(n, "getPhoneModel") || !strcmp(n, "getDevice") || !strcmp(n, "getDeviceName")) {
        *out = jv_l(jni_str("Nintendo_Switch")); return 1;
    }
    if (!strcmp(n, "GetDeviceFirmware") || !strcmp(n, "getDeviceFirmware") || !strcmp(n, "getFirmware")) {
        *out = jv_l(jni_str("1.0.0")); return 1;
    }
    if (!strcmp(n, "getCarrier")) { *out = jv_l(jni_str("Nintendo")); return 1; }
    if (!strcmp(n, "getWidth")) { *out = jv_i(MC4_WIDTH); return 1; }
    if (!strcmp(n, "getHeight")) { *out = jv_i(MC4_HEIGHT); return 1; }
    if (!strcmp(n, "getManufacture") || !strcmp(n, "getManufacturer")) { *out = jv_i(4); return 1; }
    if (!strcmp(n, "isRoaming") || !strcmp(n, "getIsRoaming") || !strcmp(n, "IsWifiEnable") ||
        !strcmp(n, "isWifiEnabled") || !strcmp(n, "isNeedGcOnFrame")) { *out = jv_z(0); return 1; }
    if (!strcmp(n, "getUniqueCode") || !strcmp(n, "createUniqueCode") || !strcmp(n, "getDemoCode")) { *out = jv_i(0); return 1; }
    if (!strcmp(n, "InitDeviceValues") || !strcmp(n, "init") || !strcmp(n, "destroy") ||
        !strcmp(n, "openBrowser") || !strcmp(n, "notifyTrophy") || !strcmp(n, "sendTrackingInfo") ||
        !strcmp(n, "showAd") || !strcmp(n, "loadAd") || !strcmp(n, "showInterstitial")) {
        *out = jv_none(); return 1;
    }
    if (unknown++ < 128)
        debugPrintf("[mc4/jni] unimplemented %s.%s%s\n", c, n, m->sig);
    return 0;
}

void mc4_jni_check(const char *stage) {
    if (unknown != reported) {
        debugPrintf("[mc4/jni] stage=%s unimplemented calls=%u\n", stage, unknown);
        reported = unknown;
    }
}

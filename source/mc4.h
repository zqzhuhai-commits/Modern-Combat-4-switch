/* SPDX-License-Identifier: MIT
 * mc4.h -- Modern Combat 4 port-specific definitions.
 * Game: com.gameloft.android.ANMP.GloftM4HM (Gameloft GL2 JNI engine).
 */
#pragma once
#include "mc4_api.h"

#define MC4_CLASS "com/gameloft/android/ANMP/GloftM4HM/MC4/"
#define MC4_PREFIX "Java_com_gameloft_android_ANMP_GloftM4HM_"
#define GL2_CLASS "com/gameloft/glf/GL2JNILib"
#define GL2_PREFIX "Java_com_gameloft_glf_GL2JNILib_"

/* Native asset/UI coordinates: the game targets a 1280x720 landscape
 * surface; the default framebuffer is the Switch screen (720p). */
#define MC4_WIDTH 1280
#define MC4_HEIGHT 720
#define MC4_OUTPUT_WIDTH 1280
#define MC4_OUTPUT_HEIGHT 720

/* Expected original library, verified before load. */
#define MC4_LIB_SHA256 "f5bcb8e6071cc1b0e09d200922dd2914e971ccbf5002981e8416481fe5b45ba1"
#define MC4_LIB_SIZE 24855648u

extern Mc4Api mc4_api;

void mc4_input_init(void);
int mc4_input_poll(void);
void mc4_input_release(void);
void mc4_audio_close(void);
void mc4_audio_pause(int pause);
int mc4_audio_write(const unsigned char *bytes, unsigned size, unsigned rate, int channels);
void mc4_jni_check(const char *stage);

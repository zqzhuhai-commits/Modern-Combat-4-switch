/* SPDX-License-Identifier: MIT
 * mc4_api.h -- Modern Combat 4 native API surface (bound from the game ELF).
 */
#pragma once
#include <stdint.h>

typedef uintptr_t (*mc4_lookup)(void *module, const char *symbol);
typedef void (*mc4_log)(const char *symbol);

typedef struct Mc4Api {
    /* lifecycle */
    int  (*on_load)(void *vm, void *reserved);
    /* GL2JNILib engine (com/gameloft/glf/GL2JNILib) */
    void (*init)(void *env, void *cls);
    void (*init_gl)(void *env, void *cls);
    int  (*step)(void *env, void *cls);
    void (*resize)(void *env, void *cls, int w, int h);
    void (*init_view_settings)(void *env, void *cls);
    void (*set_paths)(void *env, void *cls, void *a, void *b, void *c, void *d);
    void (*state_changed)(void *env, void *cls, int state);
    void (*touch)(void *env, void *cls, int type, int x, int y, int id);
    void (*key_down)(void *env, void *cls, int key);
    void (*key_up)(void *env, void *cls, int key);
    void (*keyboard)(void *env, void *cls, int key);
    void (*destroy)(void *env, void *cls);
    /* MC4 game (com/gameloft/android/ANMP/GloftM4HM/MC4) */
    int  (*game_init)(void *env, void *cls);
    void (*pause)(void *env, void *cls);
    void (*resume)(void *env, void *cls);
    void (*splash)(void *env, void *cls, int);
    void (*handle_key)(void *env, void *cls, int);
    /* auxiliary inits */
    void (*device_init)(void *env, void *cls);
    void (*data_init)(void *env, void *cls);
    void (*sutils_init)(void *env, void *cls);
    void (*iab_init)(void *env, void *cls);
    void (*social_init)(void *env, void *cls);
    void (*sendinfo_init)(void *env, void *cls);
    void (*videoview_init)(void *env, void *cls);
    /* game installer / GDRM policy (stubbed permissive) */
    int  (*installer_start)(void *env, void *cls);
    int  (*gdrm_allow)(void *env, void *cls);
} Mc4Api;

int mc4_bind(Mc4Api *api, mc4_lookup lookup, void *module, mc4_log log);
int mc4_bootstrap(const Mc4Api *a, void *vm, void *env,
                  void *(*make_string)(const char *),
                  void *(*make_class)(const char *),
                  int width, int height, int language, mc4_log log);

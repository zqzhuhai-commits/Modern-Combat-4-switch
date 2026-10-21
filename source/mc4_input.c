/* SPDX-License-Identifier: MIT. MC4 controller mapping.
 * Left stick = virtual movement stick (touch), right stick = aim cursor,
 * A = fire, ZL/ZR = aim/fire hold, X/Y = action, B/Plus = back.
 * Native touch passthrough is preserved.
 */
#include "mc4.h"
#include "jni.h"
#include <switch.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static PadState pad;
static HidTouchScreenState previous;
static int joy, joyx, joyy, aim, fire, action, grab;
static int cx = MC4_WIDTH / 2, cy = MC4_HEIGHT / 2, click;

static void touch(int type, int x, int y, int id) {
    mc4_api.touch(g_jni_env, NULL, type, x, y, id);
}
static void key(int down, int code) {
    if (down) mc4_api.key_down(g_jni_env, NULL, code);
    else mc4_api.key_up(g_jni_env, NULL, code);
}
static void held(u64 keys, u64 mask, int x, int y, int id, int *old) {
    int active = (keys & mask) != 0;
    if (active != *old) touch(active ? 1 : 0, x, y, id);
    *old = active;
}

void mc4_input_init(void) {
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);
    hidInitializeTouchScreen();
    memset(&previous, 0, sizeof previous);
}

void mc4_input_release(void) {
    if (joy) touch(0, joyx, joyy, 50);
    joy = 0;
    held(0, HidNpadButton_ZL, 636, 230, 51, &aim);   /* aim */
    held(0, HidNpadButton_ZR, 1250, 640, 52, &fire); /* fire */
    held(0, HidNpadButton_X, 640, 360, 53, &action); /* action */
    held(0, HidNpadButton_Y, 640, 360, 54, &grab);   /* grab */
    if (click) touch(0, cx, cy, 55);
    click = 0;
    for (int i = 0; i < previous.count; i++)
        touch(0, previous.touches[i].x * MC4_WIDTH / 1280,
              previous.touches[i].y * MC4_HEIGHT / 720,
              (int)previous.touches[i].finger_id);
    previous.count = 0;
}

int mc4_input_poll(void) {
    padUpdate(&pad);
    u64 keys = padGetButtons(&pad), down = padGetButtonsDown(&pad), up = padGetButtonsUp(&pad);
    if ((keys & HidNpadButton_Plus) && (keys & HidNpadButton_Minus)) return 1;
    /* BACK (Android keycode 4): B or Plus. */
    if (down & (HidNpadButton_Plus | HidNpadButton_B)) key(1, 4);
    if (up & (HidNpadButton_Plus | HidNpadButton_B)) key(0, 4);
    /* MENU (82): Minus. */
    if (down & HidNpadButton_Minus) key(1, 82);
    if (up & HidNpadButton_Minus) key(0, 82);

    HidAnalogStickState l = padGetStickPos(&pad, 0), r = padGetStickPos(&pad, 1);
    /* Left stick: virtual movement stick centred at screen left. */
    if (abs(l.x) > 6000 || abs(l.y) > 6000) {
        int x = 150 + l.x * 120 / 32768, y = 360 - l.y * 120 / 32768;
        if (x < 0) x = 0; if (x >= MC4_WIDTH) x = MC4_WIDTH - 1;
        if (y < 0) y = 0; if (y >= MC4_HEIGHT) y = MC4_HEIGHT - 1;
        if (!joy) touch(1, 150, 360, 50);
        touch(2, x, y, 50);
        joy = 1; joyx = x; joyy = y;
    } else if (joy) { touch(0, joyx, joyy, 50); joy = 0; }
    /* Right stick: aim cursor. */
    if (abs(r.x) > 6000) cx += r.x / 4000;
    if (abs(r.y) > 6000) cy -= r.y / 4000;
    if (keys & HidNpadButton_Left) cx -= 10;
    if (keys & HidNpadButton_Right) cx += 10;
    if (keys & HidNpadButton_Up) cy -= 10;
    if (keys & HidNpadButton_Down) cy += 10;
    if (cx < 0) cx = 0;
    if (cx >= MC4_WIDTH) cx = MC4_WIDTH - 1;
    if (cy < 0) cy = 0;
    if (cy >= MC4_HEIGHT) cy = MC4_HEIGHT - 1;
    /* A = fire (click). */
    if (down & HidNpadButton_A) { touch(1, cx, cy, 55); click = 1; }
    if (click && (keys & HidNpadButton_A)) touch(2, cx, cy, 55);
    if (up & HidNpadButton_A) { touch(0, cx, cy, 55); click = 0; }
    held(keys, HidNpadButton_ZL, 636, 230, 51, &aim);
    held(keys, HidNpadButton_ZR, 1250, 640, 52, &fire);
    held(keys, HidNpadButton_X, 640, 360, 53, &action);
    held(keys, HidNpadButton_Y, 640, 360, 54, &grab);

    HidTouchScreenState now = {0};
    hidGetTouchScreenStates(&now, 1);
    for (int i = 0; i < previous.count; i++) {
        int found = 0;
        for (int j = 0; j < now.count; j++)
            found |= previous.touches[i].finger_id == now.touches[j].finger_id;
        if (!found)
            touch(0, previous.touches[i].x * MC4_WIDTH / 1280,
                  previous.touches[i].y * MC4_HEIGHT / 720,
                  (int)previous.touches[i].finger_id);
    }
    for (int i = 0; i < now.count; i++) {
        int found = 0;
        for (int j = 0; j < previous.count; j++)
            found |= now.touches[i].finger_id == previous.touches[j].finger_id;
        touch(found ? 2 : 1, now.touches[i].x * MC4_WIDTH / 1280,
              now.touches[i].y * MC4_HEIGHT / 720, (int)now.touches[i].finger_id);
    }
    previous = now;
    return 0;
}

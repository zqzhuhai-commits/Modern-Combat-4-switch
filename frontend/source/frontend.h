/* SPDX-License-Identifier: MIT */
#pragma once
#include <stddef.h>
#include <stdint.h>
#define FRONTEND_VERSION "0.1.1"
#define GAME_ROOT "sdmc:/switch/shadowguardian_nx"
#define FRONTEND_TITLE_ID 0x0100F53447550000ULL
/* A candidate only: no claim of supported package/version or playable runtime. */
typedef struct {
    int apk_candidates, data_files, limited, errors;
    int library_present, library_verified;
    uint64_t data_bytes;
    char apk[768];
    char library[768], data_path[1024], library_sha256[65];
} Scan;
void scan_files(const char *root, Scan *s);
int save_report(const char *root, const Scan *s);
void draw_frontend(uint32_t *pixels, unsigned stride, int page, const Scan *s, const char *notice);

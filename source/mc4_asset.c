/* SPDX-License-Identifier: MIT
 * mc4_asset.c -- AAssetManager for Modern Combat 4.
 *
 * The game reads its expansion files (OBB) and APK assets through the
 * libandroid AAsset API. Both OBBs are STORED (uncompressed) zips, so an
 * AAsset is a window (offset,size) into the open OBB file: seek/read are
 * plain preads. The zip central directory is parsed once on first use.
 *
 * OBB lookup order: <root>/obb/main.1100...obb, <root>/main.1100...obb.
 * patch.12222...obb entries override main's. APK assets fall back to the
 * unpacked assets/ folder on the SD card via the runtime path rules.
 */
#include "rt_settings.h"
#include "util.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include "dcr_path.h"

#define OBB_MAIN "main.1100.com.gameloft.android.ANMP.GloftM4HM.obb"
#define OBB_PATCH "patch.12222.com.gameloft.android.ANMP.GloftM4HM.obb"

typedef struct {
    char name[128];
    uint64_t data_off;
    uint64_t size;
} AssetEntry;

typedef struct AAsset {
    int fd;
    uint64_t data_off;
    uint64_t size;
    uint64_t pos;
} AAsset;

static AssetEntry entries[512];
static unsigned nentries;
static int obb_fd = -1;
static int parsed;

static uint16_t rd16(const unsigned char *p) { return (uint16_t)(p[0] | (uint16_t)p[1] << 8); }
static uint32_t rd32(const unsigned char *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static void add_entry(const char *name, uint64_t data_off, uint64_t size) {
    if (nentries >= sizeof entries / sizeof entries[0]) return;
    AssetEntry *e = &entries[nentries];
    snprintf(e->name, sizeof e->name, "%s", name);
    e->data_off = data_off;
    e->size = size;
    nentries++;
}

/* Replace an existing entry (patch OBB overrides main). */
static void put_entry(const char *name, uint64_t data_off, uint64_t size) {
    for (unsigned i = 0; i < nentries; i++)
        if (!strcmp(entries[i].name, name)) {
            entries[i].data_off = data_off;
            entries[i].size = size;
            return;
        }
    add_entry(name, data_off, size);
}

static int parse_obb(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    if (fseek(f, 0, SEEK_END)) { fclose(f); return -1; }
    long flen = ftell(f);
    if (flen < 22) { fclose(f); return -1; }

    /* Locate EOCD: scan the last 64 KiB + comment from the end. */
    long scan = flen > 65536 ? 65536 : flen;
    unsigned char tail[65536];
    if (fseek(f, flen - scan, SEEK_SET) || fread(tail, 1, (size_t)scan, f) != (size_t)scan) { fclose(f); return -1; }
    int eocd = -1;
    for (long i = scan - 22; i >= 0; i--) {
        if (rd32(tail + i) == 0x06054b50) { eocd = (int)i; break; }
    }
    if (eocd < 0) { fclose(f); return -1; }
    unsigned cd_size = rd32(tail + eocd + 12);
    unsigned cd_off = rd32(tail + eocd + 16);
    if ((long)cd_off + cd_size > flen) { fclose(f); return -1; }

    unsigned char *cd = malloc(cd_size ? cd_size : 1);
    if (!cd) { fclose(f); return -1; }
    if (fseek(f, cd_off, SEEK_SET) || fread(cd, 1, cd_size, f) != cd_size) { free(cd); fclose(f); return -1; }

    unsigned pos = 0;
    while (pos + 46 <= cd_size) {
        if (rd32(cd + pos) != 0x02014b50) break;
        unsigned method = rd16(cd + pos + 10);
        unsigned csize = rd32(cd + pos + 20);
        unsigned usize = rd32(cd + pos + 24);
        unsigned nlen = rd16(cd + pos + 28);
        unsigned elen = rd16(cd + pos + 30);
        unsigned clen = rd16(cd + pos + 32);
        unsigned lho = rd32(cd + pos + 42);
        if (method == 0 && pos + 46 + nlen <= cd_size) {
            char name[128];
            if (nlen < sizeof name) {
                memcpy(name, cd + pos + 46, nlen);
                name[nlen] = 0;
                /* Data begins at local header + 30 + name_len + extra_len. */
                uint64_t data_off = (uint64_t)lho + 30;
                /* local header name/extra lengths are authoritative; for
                 * STORED entries they normally equal the CD's. */
                put_entry(name, data_off, (uint64_t)usize);
            }
        }
        pos += 46 + nlen + elen + clen;
    }
    free(cd);
    fclose(f);
    return 0;
}

static int ensure_parsed(void) {
    if (parsed) return obb_fd >= 0 ? 0 : -1;
    parsed = 1;
    char p[512];
    snprintf(p, sizeof p, "%s/obb/%s", dcr_game_root(), OBB_MAIN);
    if (access(p, R_OK)) snprintf(p, sizeof p, "%s/%s", dcr_game_root(), OBB_MAIN);
    if (parse_obb(p)) return -1;
    snprintf(p, sizeof p, "%s/obb/%s", dcr_game_root(), OBB_PATCH);
    if (access(p, R_OK)) snprintf(p, sizeof p, "%s/%s", dcr_game_root(), OBB_PATCH);
    parse_obb(p); /* patch is optional */
    obb_fd = open(p, O_RDONLY);
    debugPrintf("[asset] OBB entries=%u fd=%d\n", nentries, obb_fd);
    return obb_fd >= 0 ? 0 : -1;
}

static const AssetEntry *find_entry(const char *name) {
    if (!name) return NULL;
    if (ensure_parsed()) return NULL;
    for (unsigned i = 0; i < nentries; i++)
        if (!strcmp(entries[i].name, name)) return &entries[i];
    return NULL;
}

/* ------------------------------------------------------------- the API */
typedef struct AAssetManager AAssetManager;

AAssetManager *b_AAssetManager_fromJava(void *env, void *assetManager) {
    (void)env;
    (void)assetManager;
    /* The AAsset API is stateless here: the manager is the OBB handle. */
    return (AAssetManager *)&obb_fd;
}

AAsset *b_AAssetManager_open(AAssetManager *mgr, const char *filename, int mode) {
    (void)mgr;
    (void)mode;
    const AssetEntry *e = find_entry(filename);
    if (!e) {
        /* Fall back to unpacked assets/ on the SD card. */
        char p[640];
        const char *base = filename;
        const char *slash = strrchr(filename, '/');
        if (slash) base = slash + 1;
        snprintf(p, sizeof p, "%s/assets/%s", dcr_game_root(), base);
        FILE *f = fopen(p, "rb");
        if (!f) { debugPrintf("[asset] open missing %s\n", filename); return NULL; }
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fclose(f);
        static AAsset fallback;
        fallback.fd = open(p, O_RDONLY);
        if (fallback.fd < 0) return NULL;
        fallback.data_off = 0;
        fallback.size = (uint64_t)size;
        fallback.pos = 0;
        return &fallback;
    }
    if (obb_fd < 0) return NULL;
    static AAsset assets[8];
    static unsigned next;
    AAsset *a = &assets[next++ % 8];
    a->fd = obb_fd;
    a->data_off = e->data_off;
    a->size = e->size;
    a->pos = 0;
    return a;
}

ssize_t b_AAsset_read(AAsset *a, void *buf, size_t count) {
    if (!a || a->fd < 0) return -1;
    if (a->pos >= a->size) return 0;
    if (count > a->size - a->pos) count = (size_t)(a->size - a->pos);
    /* pread is absent from newlib-nano: emulate with lseek+read. */
off_t saved = lseek(a->fd, 0, SEEK_CUR);
off_t at = lseek(a->fd, (off_t)(a->data_off + a->pos), SEEK_SET);
ssize_t r = at < 0 ? -1 : read(a->fd, buf, count);
if (saved >= 0) lseek(a->fd, saved, SEEK_SET);
    if (r > 0) a->pos += (uint64_t)r;
    return r;
}

off_t b_AAsset_seek(AAsset *a, off_t offset, int whence) {
    if (!a || a->fd < 0) return (off_t)-1;
    int64_t target;
    if (whence == SEEK_SET) target = offset;
    else if (whence == SEEK_CUR) target = (int64_t)a->pos + offset;
    else if (whence == SEEK_END) target = (int64_t)a->size + offset;
    else return (off_t)-1;
    if (target < 0 || (uint64_t)target > a->size) return (off_t)-1;
    a->pos = (uint64_t)target;
    return (off_t)a->pos;
}

off_t b_AAsset_getLength(AAsset *a) {
    return a && a->fd >= 0 ? (off_t)a->size : (off_t)-1;
}

void b_AAsset_close(AAsset *a) {
    (void)a; /* fd is shared with the OBB; nothing to release */
}

/* Directory enumeration (rarely used by MC4): return next entry name. */
typedef struct {
    unsigned next;
    int valid;
} AAssetDir;

AAssetDir *b_AAssetManager_openDir(AAssetManager *mgr, const char *dirname) {
    (void)mgr;
    (void)dirname;
    static AAssetDir d;
    d.next = 0;
    d.valid = ensure_parsed() == 0;
    return &d;
}

const char *b_AAssetDir_getNextFileName(AAssetDir *d) {
    if (!d || !d->valid) return NULL;
    while (d->next < nentries) {
        unsigned i = d->next++;
        if (strchr(entries[i].name, '/') == NULL) return entries[i].name;
    }
    return NULL;
}

void b_AAssetDir_close(AAssetDir *d) { (void)d; }

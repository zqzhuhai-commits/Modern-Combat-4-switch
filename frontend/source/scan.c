/* SPDX-License-Identifier: MIT */
#include "frontend.h"
#include "rt_apkfind.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <errno.h>
#ifdef __SWITCH__
#include <switch/crypto/sha256.h>
#else
#define OPENSSL_API_COMPAT 0x10100000L
#include <openssl/sha.h>
#endif

static void walk(const char *path, Scan *s, int depth, int *budget) {
    DIR *d = opendir(path);
    if (!d) { if (errno != ENOENT) s->errors++; return; }
    struct dirent *e;
    while ((e = readdir(d))) {
        if (!strcmp(e->d_name,".") || !strcmp(e->d_name,"..")) continue;
        if ((*budget)-- <= 0) { s->limited = 1; break; }
        char p[1024];
        if (snprintf(p,sizeof p,"%s/%s",path,e->d_name) >= (int)sizeof p) { s->limited=1; continue; }
        struct stat st;
        /* Do not follow symlinks in host checks or future non-SD filesystems. */
        if (lstat(p,&st)) { s->errors++; continue; }
        if (S_ISREG(st.st_mode) && st.st_size > 0) { s->data_files++; s->data_bytes += st.st_size; }
        else if (S_ISDIR(st.st_mode)) {
            if (depth < 6) walk(p,s,depth+1,budget); else s->limited=1;
        }
    }
    closedir(d);
}
static void library_check(const char *path, Scan *s) {
    FILE *f=fopen(path,"rb"); if (!f) return;
    s->library_present=1;
    snprintf(s->library,sizeof s->library,"%s",path);
    unsigned char digest[32], buf[16384]; size_t n;
#ifdef __SWITCH__
    Sha256Context c; sha256ContextCreate(&c);
    while ((n=fread(buf,1,sizeof buf,f))) sha256ContextUpdate(&c,buf,n);
    sha256ContextGetHash(&c,digest);
#else
    SHA256_CTX c; SHA256_Init(&c);
    while ((n=fread(buf,1,sizeof buf,f))) SHA256_Update(&c,buf,n);
    SHA256_Final(digest,&c);
#endif
    int error=ferror(f); fclose(f);
    if (error) {s->errors++;return;}
    for (int i=0;i<32;i++) snprintf(s->library_sha256+2*i,3,"%02x",digest[i]);
    s->library_verified=!strcmp(s->library_sha256,
      "ad17a127ae438e829a23e372ddd8100f82fe6a311b0ad295b8cead69d5805ef9");
}
void scan_files(const char *root, Scan *s) {
    memset(s,0,sizeof *s);
    DIR *d=opendir(root);
    if (d) {
        struct dirent *e; int budget=512;
        while ((e=readdir(d))) {
            if (--budget < 0) { s->limited=1; break; }
            size_t n=strlen(e->d_name);
            if (n<4 || strcasecmp(e->d_name+n-4,".apk")) continue;
            char p[768]; struct stat st;
            if (snprintf(p,sizeof p,"%s/%s",root,e->d_name) >= (int)sizeof p) continue;
            if (lstat(p,&st) || !S_ISREG(st.st_mode)) continue;
            if (rt_zip_has(p,"AndroidManifest.xml") && rt_zip_has(p,"lib/armeabi-v7a/libshadowguardian.so")) {
                s->apk_candidates++;
                if (!s->apk[0] || strcmp(p,s->apk)<0) snprintf(s->apk,sizeof s->apk,"%s",p);
            }
        }
        closedir(d);
    } else if (errno != ENOENT) s->errors++;
    char path[1024];
    snprintf(path,sizeof path,"%s/libshadowguardian.so",root);library_check(path,s);
    if (!s->library_present) {
        snprintf(path,sizeof path,"%s/lib/armeabi-v7a/libshadowguardian.so",root);library_check(path,s);
    }
    const char *locations[]={"gameloft/games/GloftSGHP","Gameloft/Games/GloftSGHP"};
    for (unsigned i=0;i<2;i++) {
        if (snprintf(path,sizeof path,"%s/%s",root,locations[i]) >= (int)sizeof path) {s->errors++;return;}
        struct stat st;
        if (!stat(path,&st) && S_ISDIR(st.st_mode)) {
            snprintf(s->data_path,sizeof s->data_path,"%s",path);
            int budget=4096; walk(path,s,0,&budget);break;
        }
    }
}
int save_report(const char *root, const Scan *s) {
    /* Only creates frontend.log. No APK, game data, or save modification. */
    char path[1024];
    if (snprintf(path,sizeof path,"%s/frontend.log",root) >= (int)sizeof path) return -1;
    FILE *f=fopen(path,"w"); if (!f) return -1;
    int n=fprintf(f,"Shadow Guardian Frontend %s\nMode: preparation only; game launch unavailable\n"
      "APK candidates: %d\nSelected candidate: %s\nData files: %d\nData bytes: %llu\n"
      "Scan bounded/incomplete: %d\nIO errors: %d\n"
      "Checks inspect ZIP entry names and nonempty data files only.\n"
      "Package/version, CRC, ELF exports, resources and gameplay are NOT validated.\n",
      FRONTEND_VERSION,s->apk_candidates,s->apk,s->data_files,(unsigned long long)s->data_bytes,s->limited,s->errors);
    if (n>=0) n=fprintf(f,"Library present: %d\nKnown SHA256 match: %d\nLibrary SHA256: %s\nData path: %s\nKnown library match does NOT establish game compatibility.\n",
        s->library_present,s->library_verified,s->library_sha256,s->data_path);
    int fail=n<0; if (fclose(f)) fail=1; return fail ? -1:0;
}

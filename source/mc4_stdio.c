/* SPDX-License-Identifier: MIT
 * mc4_stdio.c -- Modern Combat 4 stdio compatibility.
 * Android 2.3's stdio.h defines fpos_t as off_t (32-bit long on ARM);
 * do not hand a guest fpos_t* to newlib (its off_t is 64 bits).
 */
#include <stdint.h>
#include <stdio.h>

extern int32_t b_ftello(void *fp);
extern int b_fseeko(void *fp, int32_t off, int whence);
extern void b_set_errno(int linux_errno);

/* Unique address for this permanently loaded game module's C++ destructors. */
void *b___dso_handle = &b___dso_handle;

int b_fgetpos(void *fp, int32_t *pos) {
    if (!pos) { b_set_errno(22); return -1; }
    int32_t value = b_ftello(fp);
    if (value < 0) return -1;
    *pos = value;
    return 0;
}

int b_fsetpos(void *fp, const int32_t *pos) {
    if (!pos) { b_set_errno(22); return -1; }
    return b_fseeko(fp, *pos, SEEK_SET);
}
/* SPDX-License-Identifier: MIT
 * mc4_stdio.c -- Modern Combat 4 stdio compatibility.
 * Android 2.3's stdio.h defines fpos_t as off_t (32-bit long on ARM);
 * do not hand a guest fpos_t* to newlib (its off_t is 64 bits).
 */
#include <stdint.h>
#include <stdio.h>


/* newlib-nano ships no umask; the game's dlopen expects one. */
#include <sys/types.h>
mode_t umask(mode_t m) { (void)m; return 0; }

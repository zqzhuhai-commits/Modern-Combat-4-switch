/* SPDX-License-Identifier: MIT. MC4 GL compatibility layer.
 * ATC textures are software-decoded (Mesa has no ATC support); viewport and
 * scissor are tracked so framebuffer switches restore the game's rects.
 * Output is 1280x720: the game's native coordinates are 1:1 with the screen.
 */
#include "mc4_atc.h"
#include "mc4.h"
#include "gl_layer.h"
#include "util.h"
#include <GLES2/gl2.h>
#include <stdlib.h>
#include <string.h>

static void (*compressed_upload)(GLenum, GLint, GLenum, GLsizei, GLsizei,
                                 GLint, GLsizei, const void *);
static unsigned successes, failures;
static void mc4_compressed_upload(GLenum target, GLint level, GLenum format,
        GLsizei width, GLsizei height, GLint border, GLsizei bytes, const void *data) {
    if (format != MC4_ATC_RGB && format != MC4_ATC_EXPLICIT_ALPHA &&
        format != MC4_ATC_INTERPOLATED_ALPHA) {
        compressed_upload(target, level, format, width, height, border, bytes, data);
        return;
    }
    if (target != GL_TEXTURE_2D || level < 0 || border != 0) {
        compressed_upload(target, level, format, width, height, border, bytes, data);
        return;
    }
    uint8_t *rgba = NULL;
    int result = mc4_atc_decode(format, width, height, bytes, data, &rgba);
    if (result) {
        if (failures++ < 16)
            debugPrintf("[mc4/atc] rejected format=0x%x %dx%d bytes=%d result=%d\n",
                        format, width, height, bytes, result);
        return;
    }
    GLint unpack = 4;
    glGetIntegerv(0x2305 /* GL_UNPACK_ALIGNMENT */, &unpack);
    glPixelStorei(0x2305 /* GL_UNPACK_ALIGNMENT */, 1);
    glTexImage2D(target, level, GL_RGBA, width, height, border,
                 GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    glPixelStorei(0x2305 /* GL_UNPACK_ALIGNMENT */, unpack);
    free(rgba);
    if (successes++ < 16)
        debugPrintf("[mc4/atc] decoded format=0x%x level=%d %dx%d bytes=%d -> RGBA8\n",
                    format, level, width, height, bytes);
}

static GLint viewport[4] = {0, 0, MC4_WIDTH, MC4_HEIGHT};
static GLint scissor_box[4] = {0, 0, MC4_WIDTH, MC4_HEIGHT};
static void (*real_viewport)(GLint, GLint, GLsizei, GLsizei);
static void (*real_scissor)(GLint, GLint, GLsizei, GLsizei);
static void (*real_bind_framebuffer)(GLenum, GLuint);
static void (*real_get_integer)(GLenum, GLint *);
static void apply_rect(void (*fn)(GLint, GLint, GLsizei, GLsizei), const GLint *r) {
    fn(r[0], r[1], r[2], r[3]);
}
static void mc4_viewport(GLint x, GLint y, GLsizei w, GLsizei h) {
    if (w < 0 || h < 0) { real_viewport(x, y, w, h); return; }
    viewport[0] = x; viewport[1] = y; viewport[2] = w; viewport[3] = h;
    apply_rect(real_viewport, viewport);
}
static void mc4_scissor(GLint x, GLint y, GLsizei w, GLsizei h) {
    if (w < 0 || h < 0) { real_scissor(x, y, w, h); return; }
    scissor_box[0] = x; scissor_box[1] = y; scissor_box[2] = w; scissor_box[3] = h;
    apply_rect(real_scissor, scissor_box);
}
static void mc4_bind_framebuffer(GLenum target, GLuint fbo) {
    real_bind_framebuffer(target, fbo);
    if (target != GL_FRAMEBUFFER) return;
    if (real_viewport) apply_rect(real_viewport, viewport);
    if (real_scissor) apply_rect(real_scissor, scissor_box);
}
static void mc4_get_integer(GLenum name, GLint *value) {
    if (value && name == GL_VIEWPORT) memcpy(value, viewport, sizeof viewport);
    else if (value && name == GL_SCISSOR_BOX) memcpy(value, scissor_box, sizeof scissor_box);
    else real_get_integer(name, value);
}

uintptr_t port_gl_wrap(const char *name, uintptr_t real) {
    if (!strcmp(name, "glCompressedTexImage2D") && real) {
        compressed_upload = (void *)real;
        return (uintptr_t)mc4_compressed_upload;
    }
#define WRAP(name_, slot_, fn_) \
    if (!strcmp(name, name_) && real) { slot_ = (void *)real; return (uintptr_t)fn_; }
    WRAP("glViewport", real_viewport, mc4_viewport)
    WRAP("glScissor", real_scissor, mc4_scissor)
    WRAP("glBindFramebuffer", real_bind_framebuffer, mc4_bind_framebuffer)
    WRAP("glGetIntegerv", real_get_integer, mc4_get_integer)
#undef WRAP
    return 0;
}

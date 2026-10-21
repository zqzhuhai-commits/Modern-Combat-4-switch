#ifndef GLES2_GL2_H
#define GLES2_GL2_H
#include <stdint.h>
#include <stddef.h>
typedef int32_t GLint;
typedef uint32_t GLuint;
typedef uint8_t GLboolean;
typedef float GLfloat;
typedef int GLsizei;
typedef void GLvoid;
typedef char GLchar;
typedef short GLshort;
typedef unsigned char GLubyte;
typedef int GLenum;
typedef int GLbitfield;
typedef float GLclampf;
typedef double GLclampd;
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;
typedef uint64_t GLuint64;
typedef void *GLsync;
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_FALSE 0
#define GL_TRUE 1
#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406
#define GL_FIXED 0x1407
#define GL_DEPTH_COMPONENT 0x1902
#define GL_RED 0x1903
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_LUMINANCE 0x1909
#define GL_LUMINANCE_ALPHA 0x190A
#define GL_ALPHA 0x1906
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_REPEAT 0x2901
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE0 0x84C0
#define GL_BLEND 0x0BE2
#define GL_CULL_FACE 0x0B44
#define GL_DEPTH_TEST 0x0B71
#define GL_DITHER 0x0BD0
#define GL_SCISSOR_TEST 0x0C11
#define GL_STENCIL_TEST 0x0B90
#define GL_FRAMEBUFFER 0x8D40
#define GL_VIEWPORT 0x0BA2
#define GL_COLOR_WRITEMASK 0x0C23
#define GL_LINES 0x0001
#define GL_ARRAY_BUFFER_BINDING 0x8894
#define GL_FRAMEBUFFER_BINDING 0x8CA6
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED 0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE 0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE 0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE 0x8625
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_VERTEX_ATTRIB_ARRAY_POINTER 0x8645
#define GL_VENDOR 0x1F00
#define GL_RENDERER 0x1F01
#define GL_VERSION 0x1F02
#define GL_EXTENSIONS 0x1F03
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A3
#define GL_SCISSOR_BOX 0x0C10
#define GL_COLOR_CLEAR_VALUE 0x0C22
#define GL_NO_ERROR 0
#define GL_TRIANGLES 0x0004
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING 0x8CAA
#define GL_DRAW_FRAMEBUFFER_BINDING 0x8CAB
#define GL_PIXEL_PACK_BUFFER 0x88EB
#define GL_PIXEL_UNPACK_BUFFER 0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING 0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING 0x88EF
#define GL_PACK_ALIGNMENT 0x0CF5
#define GL_PACK_ROW_LENGTH 0x0D02
#define GL_PACK_SKIP_ROWS 0x0D03
#define GL_PACK_SKIP_PIXELS 0x0D04
#define GL_COMPRESSED_TEXTURE_FORMATS 0x86A4
#define GL_ALREADY_SIGNALED 0x911A
#define GL_TIMEOUT_EXPIRED 0x911B
#define GL_CONDITION_SATISFIED 0x911C
#define GL_SYNC_FLUSH_COMMANDS_BIT 0x00000001
#define GL_SYNC_GPU_COMMANDS_COMPLETE 0x9117
#define GL_RENDERBUFFER 0x8D41
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_SHADER_SOURCE_LENGTH 0x8B88
#define GL_CURRENT_PROGRAM 0x8B8D
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STREAM_DRAW 0x88E0
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_SHADER_TYPE 0x8B4F
#define GL_DELETE_STATUS 0x8B80
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#ifdef __cplusplus
extern "C" {
#endif
void glClear(GLbitfield mask);
void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
void glEnable(GLenum cap);
void glDisable(GLenum cap);
void glClearStencil(GLint s);
void glClearDepthf(GLclampf depth);
void glDepthMask(GLboolean flag);
void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void glBlendFunc(GLenum sfactor, GLenum dfactor);
void glBlendEquation(GLenum mode);
void glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
void glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
void glDepthFunc(GLenum func);
void glStencilFunc(GLenum func, GLint ref, GLuint mask);
void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass);
void glFrontFace(GLenum mode);
void glCullFace(GLenum mode);
void glPixelStorei(GLenum pname, GLint param);
void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
void glDrawArrays(GLenum mode, GLint first, GLsizei count);
void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);
void glFinish(void);
void glFlush(void);
void glGenTextures(GLsizei n, GLuint *textures);
void glDeleteTextures(GLsizei n, const GLuint *textures);
void glBindTexture(GLenum target, GLuint texture);
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
void glTexParameteri(GLenum target, GLenum pname, GLint param);
void glActiveTexture(GLenum texture);
void glBindFramebuffer(GLenum target, GLuint framebuffer);
void glBindRenderbuffer(GLenum target, GLuint renderbuffer);
GLenum glCheckFramebufferStatus(GLenum target);
void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers);
void glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers);
void glGenFramebuffers(GLsizei n, GLuint *framebuffers);
void glGenRenderbuffers(GLsizei n, GLuint *renderbuffers);
void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
// Shader/program functions
GLuint glCreateShader(GLenum type);
void glDeleteShader(GLuint shader);
void glShaderSource(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
void glCompileShader(GLuint shader);
void glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
void glGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei *length, GLchar *infolog);
GLuint glCreateProgram(void);
void glDeleteProgram(GLuint program);
void glAttachShader(GLuint program, GLuint shader);
void glDetachShader(GLuint program, GLuint shader);
void glLinkProgram(GLuint program);
void glUseProgram(GLuint program);
void glGetProgramiv(GLuint program, GLenum pname, GLint *params);
void glGetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei *length, GLchar *infolog);
GLint glGetAttribLocation(GLuint program, const GLchar *name);
void glBindAttribLocation(GLuint program, GLuint index, const GLchar *name);
GLint glGetUniformLocation(GLuint program, const GLchar *name);
void glUniform1f(GLint location, GLfloat x);
void glUniform2f(GLint location, GLfloat x, GLfloat y);
void glUniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z);
void glUniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void glUniform1i(GLint location, GLint x);
void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
// Buffer functions
void glGenBuffers(GLsizei n, GLuint *buffers);
void glDeleteBuffers(GLsizei n, const GLuint *buffers);
void glBindBuffer(GLenum target, GLuint buffer);
void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
void glVertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *ptr);
void glEnableVertexAttribArray(GLuint indx);
void glDisableVertexAttribArray(GLuint indx);
void glVertexAttrib1f(GLuint indx, GLfloat x);
void glVertexAttrib2f(GLuint indx, GLfloat x, GLfloat y);
void glVertexAttrib3f(GLuint indx, GLfloat x, GLfloat y, GLfloat z);
void glVertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
// Misc
void glGetIntegerv(GLenum pname, GLint *params);
void glGetFloatv(GLenum pname, GLfloat *params);
void glGetBooleanv(GLenum pname, GLboolean *params);
const GLubyte *glGetString(GLenum name);
GLenum glGetError(void);
void glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params);
void glGetVertexAttribPointerv(GLuint index, GLenum pname, void **pointer);
GLboolean glIsEnabled(GLenum cap);
void glHint(GLenum target, GLenum mode);
void glLineWidth(GLfloat width);
void glSampleCoverage(GLclampf value, GLboolean invert);
#ifdef __cplusplus
}
#endif
#endif

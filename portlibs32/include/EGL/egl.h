#ifndef EGL_EGL_H
#define EGL_EGL_H
#include <stdint.h>
#include <stddef.h>
typedef void *EGLDisplay;
typedef void *EGLConfig;
typedef void *EGLSurface;
typedef void *EGLContext;
typedef int EGLint;
typedef unsigned int EGLBoolean;
typedef unsigned int EGLenum;
typedef void *EGLNativeDisplayType;
typedef void *EGLNativeWindowType;
typedef void *EGLNativePixmapType;
typedef char EGLchar;
typedef void (*__eglMustCastToProperFunctionPointerType)(void);
#define EGL_NO_CONTEXT ((EGLContext)0)
#define EGL_NO_SURFACE ((EGLSurface)0)
#define EGL_DEFAULT_DISPLAY ((EGLDisplay)0)
#define EGL_TRUE 1
#define EGL_FALSE 0
#define EGL_WIDTH 0x3050
#define EGL_HEIGHT 0x3051
#define EGL_NONE 0x3038
#define EGL_DONT_CARE 0x3038
#define EGL_RED_SIZE 0x3024
#define EGL_GREEN_SIZE 0x3023
#define EGL_BLUE_SIZE 0x3022
#define EGL_ALPHA_SIZE 0x3021
#define EGL_DEPTH_SIZE 0x3025
#define EGL_STENCIL_SIZE 0x3026
#define EGL_BUFFER_SIZE 0x3020
#define EGL_CONFIG_CAVEAT 0x3027
#define EGL_CONFIG_ID 0x3028
#define EGL_CONFORMANT 0x3042
#define EGL_LEVEL 0x3029
#define EGL_MAX_PBUFFER_WIDTH 0x302A
#define EGL_MAX_PBUFFER_HEIGHT 0x302B
#define EGL_MAX_PBUFFER_PIXELS 0x302C
#define EGL_NATIVE_RENDERABLE 0x302D
#define EGL_NATIVE_VISUAL_ID 0x302E
#define EGL_NATIVE_VISUAL_TYPE 0x3030
#define EGL_PRESERVED_RESOURCES 0x3031
#define EGL_SAMPLES 0x3035
#define EGL_SAMPLE_BUFFERS 0x3032
#define EGL_SURFACE_TYPE 0x3033
#define EGL_TRANSPARENT_TYPE 0x3034
#define EGL_TRANSPARENT_RED_VALUE 0x3037
#define EGL_TRANSPARENT_GREEN_VALUE 0x3038
#define EGL_TRANSPARENT_BLUE_VALUE 0x3039
#define EGL_BIND_TO_TEXTURE_RGB 0x3039
#define EGL_BIND_TO_TEXTURE_RGBA 0x303A
#define EGL_MIN_SWAP_INTERVAL 0x303B
#define EGL_MAX_SWAP_INTERVAL 0x303C
#define EGL_LUMINANCE_SIZE 0x303D
#define EGL_ALPHA_MASK_SIZE 0x303E
#define EGL_COLOR_BUFFER_TYPE 0x303F
#define EGL_RENDERABLE_TYPE 0x3040
#define EGL_MATCH_NATIVE_PIXMAP 0x3041
#define EGL_SINGLE_BUFFER 0x3085
#define EGL_SWAP_BEHAVIOR 0x3090
#define EGL_TRANSPARENT_RGB 0x3052
#define EGL_VG_COLORSPACE_LINEAR 0x308A
#define EGL_VG_ALPHA_FORMAT_NONPRE 0x308B
#define EGL_OPENGL_ES_BIT 0x0001
#define EGL_OPENVG_BIT 0x0002
#define EGL_OPENGL_ES2_BIT 0x0004
#define EGL_WINDOW_BIT 0x0004
#define EGL_OPENGL_BIT 0x0008
#define EGL_VG_COLORSPACE_sRGB 0x3087
#define EGL_VG_ALPHA_FORMAT_PRE 0x3088
#define EGL_TEXTURE_2D 0x30B1
#define EGL_TEXTURE_RGB 0x30BA
#define EGL_TEXTURE_RGBA 0x30B2
#define EGL_TEXTURE_TYPE_2D 0x30B3
#define EGL_NO_TEXTURE 0x305C
#define EGL_BACK_BUFFER 0x3084
#define EGL_EXTENSIONS 0x3055
#define EGL_VENDOR 0x3053
#define EGL_VERSION 0x3054
#define EGL_CLIENT_APIS 0x308D
#define EGL_CONTEXT_CLIENT_VERSION 0x3098
#ifdef __cplusplus
extern "C" {
#endif
EGLDisplay eglGetDisplay(EGLDisplay display);
EGLBoolean eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor);
EGLBoolean eglTerminate(EGLDisplay dpy);
EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list);
EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);
EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface);
EGLBoolean eglDestroyContext(EGLDisplay dpy, EGLContext ctx);
const char *eglQueryString(EGLDisplay dpy, EGLint name);
EGLDisplay eglGetCurrentDisplay(void);
EGLSurface eglGetCurrentSurface(EGLint readdraw);
EGLContext eglGetCurrentContext(void);
EGLBoolean eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);
#define EGL_OPENGL_ES_API 0x30A0
EGLBoolean eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
EGLBoolean eglBindAPI(EGLenum api);
EGLenum eglQueryAPI(void);
void eglWaitGL(void);
void eglWaitNative(EGLint engine);
EGLSurface eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
EGLSurface eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list);
EGLBoolean eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
EGLBoolean eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
EGLBoolean eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value);
EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval);
EGLint eglGetError(void);
__eglMustCastToProperFunctionPointerType eglGetProcAddress(const char *procname);
#ifdef __cplusplus
}
#endif
#endif
EGLBoolean eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);

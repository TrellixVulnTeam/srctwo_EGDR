// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file is auto-generated from
// ui/gl/generate_bindings.py
// It's formatted by clang-format using chromium coding style:
//    clang-format -i -style=chromium filename
// DO NOT EDIT!

#ifndef UI_GL_GL_BINDINGS_AUTOGEN_EGL_H_
#define UI_GL_GL_BINDINGS_AUTOGEN_EGL_H_

#include <string>

namespace gl {

class GLContext;

typedef EGLBoolean(GL_BINDING_CALL* eglBindAPIProc)(EGLenum api);
typedef EGLBoolean(GL_BINDING_CALL* eglBindTexImageProc)(EGLDisplay dpy,
                                                         EGLSurface surface,
                                                         EGLint buffer);
typedef EGLBoolean(GL_BINDING_CALL* eglChooseConfigProc)(
    EGLDisplay dpy,
    const EGLint* attrib_list,
    EGLConfig* configs,
    EGLint config_size,
    EGLint* num_config);
typedef EGLint(GL_BINDING_CALL* eglClientWaitSyncKHRProc)(EGLDisplay dpy,
                                                          EGLSyncKHR sync,
                                                          EGLint flags,
                                                          EGLTimeKHR timeout);
typedef EGLBoolean(GL_BINDING_CALL* eglCopyBuffersProc)(
    EGLDisplay dpy,
    EGLSurface surface,
    EGLNativePixmapType target);
typedef EGLContext(GL_BINDING_CALL* eglCreateContextProc)(
    EGLDisplay dpy,
    EGLConfig config,
    EGLContext share_context,
    const EGLint* attrib_list);
typedef EGLImageKHR(GL_BINDING_CALL* eglCreateImageKHRProc)(
    EGLDisplay dpy,
    EGLContext ctx,
    EGLenum target,
    EGLClientBuffer buffer,
    const EGLint* attrib_list);
typedef EGLSurface(GL_BINDING_CALL* eglCreatePbufferFromClientBufferProc)(
    EGLDisplay dpy,
    EGLenum buftype,
    void* buffer,
    EGLConfig config,
    const EGLint* attrib_list);
typedef EGLSurface(GL_BINDING_CALL* eglCreatePbufferSurfaceProc)(
    EGLDisplay dpy,
    EGLConfig config,
    const EGLint* attrib_list);
typedef EGLSurface(GL_BINDING_CALL* eglCreatePixmapSurfaceProc)(
    EGLDisplay dpy,
    EGLConfig config,
    EGLNativePixmapType pixmap,
    const EGLint* attrib_list);
typedef EGLStreamKHR(GL_BINDING_CALL* eglCreateStreamKHRProc)(
    EGLDisplay dpy,
    const EGLint* attrib_list);
typedef EGLBoolean(
    GL_BINDING_CALL* eglCreateStreamProducerD3DTextureNV12ANGLEProc)(
    EGLDisplay dpy,
    EGLStreamKHR stream,
    EGLAttrib* attrib_list);
typedef EGLSyncKHR(GL_BINDING_CALL* eglCreateSyncKHRProc)(
    EGLDisplay dpy,
    EGLenum type,
    const EGLint* attrib_list);
typedef EGLSurface(GL_BINDING_CALL* eglCreateWindowSurfaceProc)(
    EGLDisplay dpy,
    EGLConfig config,
    EGLNativeWindowType win,
    const EGLint* attrib_list);
typedef EGLBoolean(GL_BINDING_CALL* eglDestroyContextProc)(EGLDisplay dpy,
                                                           EGLContext ctx);
typedef EGLBoolean(GL_BINDING_CALL* eglDestroyImageKHRProc)(EGLDisplay dpy,
                                                            EGLImageKHR image);
typedef EGLBoolean(GL_BINDING_CALL* eglDestroyStreamKHRProc)(
    EGLDisplay dpy,
    EGLStreamKHR stream);
typedef EGLBoolean(GL_BINDING_CALL* eglDestroySurfaceProc)(EGLDisplay dpy,
                                                           EGLSurface surface);
typedef EGLBoolean(GL_BINDING_CALL* eglDestroySyncKHRProc)(EGLDisplay dpy,
                                                           EGLSyncKHR sync);
typedef EGLBoolean(GL_BINDING_CALL* eglGetConfigAttribProc)(EGLDisplay dpy,
                                                            EGLConfig config,
                                                            EGLint attribute,
                                                            EGLint* value);
typedef EGLBoolean(GL_BINDING_CALL* eglGetConfigsProc)(EGLDisplay dpy,
                                                       EGLConfig* configs,
                                                       EGLint config_size,
                                                       EGLint* num_config);
typedef EGLContext(GL_BINDING_CALL* eglGetCurrentContextProc)(void);
typedef EGLDisplay(GL_BINDING_CALL* eglGetCurrentDisplayProc)(void);
typedef EGLSurface(GL_BINDING_CALL* eglGetCurrentSurfaceProc)(EGLint readdraw);
typedef EGLDisplay(GL_BINDING_CALL* eglGetDisplayProc)(
    EGLNativeDisplayType display_id);
typedef EGLint(GL_BINDING_CALL* eglGetErrorProc)(void);
typedef EGLDisplay(GL_BINDING_CALL* eglGetPlatformDisplayEXTProc)(
    EGLenum platform,
    void* native_display,
    const EGLint* attrib_list);
typedef __eglMustCastToProperFunctionPointerType(
    GL_BINDING_CALL* eglGetProcAddressProc)(const char* procname);
typedef EGLBoolean(GL_BINDING_CALL* eglGetSyncAttribKHRProc)(EGLDisplay dpy,
                                                             EGLSyncKHR sync,
                                                             EGLint attribute,
                                                             EGLint* value);
typedef EGLBoolean(GL_BINDING_CALL* eglGetSyncValuesCHROMIUMProc)(
    EGLDisplay dpy,
    EGLSurface surface,
    EGLuint64CHROMIUM* ust,
    EGLuint64CHROMIUM* msc,
    EGLuint64CHROMIUM* sbc);
typedef EGLBoolean(GL_BINDING_CALL* eglImageFlushExternalEXTProc)(
    EGLDisplay dpy,
    EGLImageKHR image,
    const EGLAttrib* attrib_list);
typedef EGLBoolean(GL_BINDING_CALL* eglInitializeProc)(EGLDisplay dpy,
                                                       EGLint* major,
                                                       EGLint* minor);
typedef EGLBoolean(GL_BINDING_CALL* eglMakeCurrentProc)(EGLDisplay dpy,
                                                        EGLSurface draw,
                                                        EGLSurface read,
                                                        EGLContext ctx);
typedef EGLBoolean(GL_BINDING_CALL* eglPostSubBufferNVProc)(EGLDisplay dpy,
                                                            EGLSurface surface,
                                                            EGLint x,
                                                            EGLint y,
                                                            EGLint width,
                                                            EGLint height);
typedef EGLint(GL_BINDING_CALL* eglProgramCacheGetAttribANGLEProc)(
    EGLDisplay dpy,
    EGLenum attrib);
typedef void(GL_BINDING_CALL* eglProgramCachePopulateANGLEProc)(
    EGLDisplay dpy,
    const void* key,
    EGLint keysize,
    const void* binary,
    EGLint binarysize);
typedef void(GL_BINDING_CALL* eglProgramCacheQueryANGLEProc)(
    EGLDisplay dpy,
    EGLint index,
    void* key,
    EGLint* keysize,
    void* binary,
    EGLint* binarysize);
typedef EGLint(GL_BINDING_CALL* eglProgramCacheResizeANGLEProc)(EGLDisplay dpy,
                                                                EGLint limit,
                                                                EGLenum mode);
typedef EGLenum(GL_BINDING_CALL* eglQueryAPIProc)(void);
typedef EGLBoolean(GL_BINDING_CALL* eglQueryContextProc)(EGLDisplay dpy,
                                                         EGLContext ctx,
                                                         EGLint attribute,
                                                         EGLint* value);
typedef EGLBoolean(GL_BINDING_CALL* eglQueryStreamKHRProc)(EGLDisplay dpy,
                                                           EGLStreamKHR stream,
                                                           EGLenum attribute,
                                                           EGLint* value);
typedef EGLBoolean(GL_BINDING_CALL* eglQueryStreamu64KHRProc)(
    EGLDisplay dpy,
    EGLStreamKHR stream,
    EGLenum attribute,
    EGLuint64KHR* value);
typedef const char*(GL_BINDING_CALL* eglQueryStringProc)(EGLDisplay dpy,
                                                         EGLint name);
typedef EGLBoolean(GL_BINDING_CALL* eglQuerySurfaceProc)(EGLDisplay dpy,
                                                         EGLSurface surface,
                                                         EGLint attribute,
                                                         EGLint* value);
typedef EGLBoolean(GL_BINDING_CALL* eglQuerySurfacePointerANGLEProc)(
    EGLDisplay dpy,
    EGLSurface surface,
    EGLint attribute,
    void** value);
typedef EGLBoolean(GL_BINDING_CALL* eglReleaseTexImageProc)(EGLDisplay dpy,
                                                            EGLSurface surface,
                                                            EGLint buffer);
typedef EGLBoolean(GL_BINDING_CALL* eglReleaseThreadProc)(void);
typedef EGLBoolean(GL_BINDING_CALL* eglStreamAttribKHRProc)(EGLDisplay dpy,
                                                            EGLStreamKHR stream,
                                                            EGLenum attribute,
                                                            EGLint value);
typedef EGLBoolean(GL_BINDING_CALL* eglStreamConsumerAcquireKHRProc)(
    EGLDisplay dpy,
    EGLStreamKHR stream);
typedef EGLBoolean(
    GL_BINDING_CALL* eglStreamConsumerGLTextureExternalAttribsNVProc)(
    EGLDisplay dpy,
    EGLStreamKHR stream,
    EGLAttrib* attrib_list);
typedef EGLBoolean(GL_BINDING_CALL* eglStreamConsumerGLTextureExternalKHRProc)(
    EGLDisplay dpy,
    EGLStreamKHR stream);
typedef EGLBoolean(GL_BINDING_CALL* eglStreamConsumerReleaseKHRProc)(
    EGLDisplay dpy,
    EGLStreamKHR stream);
typedef EGLBoolean(GL_BINDING_CALL* eglStreamPostD3DTextureNV12ANGLEProc)(
    EGLDisplay dpy,
    EGLStreamKHR stream,
    void* texture,
    const EGLAttrib* attrib_list);
typedef EGLBoolean(GL_BINDING_CALL* eglSurfaceAttribProc)(EGLDisplay dpy,
                                                          EGLSurface surface,
                                                          EGLint attribute,
                                                          EGLint value);
typedef EGLBoolean(GL_BINDING_CALL* eglSwapBuffersProc)(EGLDisplay dpy,
                                                        EGLSurface surface);
typedef EGLBoolean(GL_BINDING_CALL* eglSwapBuffersWithDamageKHRProc)(
    EGLDisplay dpy,
    EGLSurface surface,
    EGLint* rects,
    EGLint n_rects);
typedef EGLBoolean(GL_BINDING_CALL* eglSwapIntervalProc)(EGLDisplay dpy,
                                                         EGLint interval);
typedef EGLBoolean(GL_BINDING_CALL* eglTerminateProc)(EGLDisplay dpy);
typedef EGLBoolean(GL_BINDING_CALL* eglWaitClientProc)(void);
typedef EGLBoolean(GL_BINDING_CALL* eglWaitGLProc)(void);
typedef EGLBoolean(GL_BINDING_CALL* eglWaitNativeProc)(EGLint engine);
typedef EGLint(GL_BINDING_CALL* eglWaitSyncKHRProc)(EGLDisplay dpy,
                                                    EGLSyncKHR sync,
                                                    EGLint flags);

struct ExtensionsEGL {
  bool b_EGL_EXT_platform_base;
  bool b_EGL_ANGLE_d3d_share_handle_client_buffer;
  bool b_EGL_ANGLE_program_cache_control;
  bool b_EGL_ANGLE_query_surface_pointer;
  bool b_EGL_ANGLE_stream_producer_d3d_texture_nv12;
  bool b_EGL_ANGLE_surface_d3d_texture_2d_share_handle;
  bool b_EGL_CHROMIUM_sync_control;
  bool b_EGL_EXT_image_flush_external;
  bool b_EGL_KHR_fence_sync;
  bool b_EGL_KHR_gl_texture_2D_image;
  bool b_EGL_KHR_image;
  bool b_EGL_KHR_image_base;
  bool b_EGL_KHR_stream;
  bool b_EGL_KHR_stream_consumer_gltexture;
  bool b_EGL_KHR_swap_buffers_with_damage;
  bool b_EGL_KHR_wait_sync;
  bool b_EGL_NV_post_sub_buffer;
  bool b_EGL_NV_stream_consumer_gltexture_yuv;
  bool b_GL_CHROMIUM_egl_khr_fence_sync_hack;
};

struct ProcsEGL {
  eglBindAPIProc eglBindAPIFn;
  eglBindTexImageProc eglBindTexImageFn;
  eglChooseConfigProc eglChooseConfigFn;
  eglClientWaitSyncKHRProc eglClientWaitSyncKHRFn;
  eglCopyBuffersProc eglCopyBuffersFn;
  eglCreateContextProc eglCreateContextFn;
  eglCreateImageKHRProc eglCreateImageKHRFn;
  eglCreatePbufferFromClientBufferProc eglCreatePbufferFromClientBufferFn;
  eglCreatePbufferSurfaceProc eglCreatePbufferSurfaceFn;
  eglCreatePixmapSurfaceProc eglCreatePixmapSurfaceFn;
  eglCreateStreamKHRProc eglCreateStreamKHRFn;
  eglCreateStreamProducerD3DTextureNV12ANGLEProc
      eglCreateStreamProducerD3DTextureNV12ANGLEFn;
  eglCreateSyncKHRProc eglCreateSyncKHRFn;
  eglCreateWindowSurfaceProc eglCreateWindowSurfaceFn;
  eglDestroyContextProc eglDestroyContextFn;
  eglDestroyImageKHRProc eglDestroyImageKHRFn;
  eglDestroyStreamKHRProc eglDestroyStreamKHRFn;
  eglDestroySurfaceProc eglDestroySurfaceFn;
  eglDestroySyncKHRProc eglDestroySyncKHRFn;
  eglGetConfigAttribProc eglGetConfigAttribFn;
  eglGetConfigsProc eglGetConfigsFn;
  eglGetCurrentContextProc eglGetCurrentContextFn;
  eglGetCurrentDisplayProc eglGetCurrentDisplayFn;
  eglGetCurrentSurfaceProc eglGetCurrentSurfaceFn;
  eglGetDisplayProc eglGetDisplayFn;
  eglGetErrorProc eglGetErrorFn;
  eglGetPlatformDisplayEXTProc eglGetPlatformDisplayEXTFn;
  eglGetProcAddressProc eglGetProcAddressFn;
  eglGetSyncAttribKHRProc eglGetSyncAttribKHRFn;
  eglGetSyncValuesCHROMIUMProc eglGetSyncValuesCHROMIUMFn;
  eglImageFlushExternalEXTProc eglImageFlushExternalEXTFn;
  eglInitializeProc eglInitializeFn;
  eglMakeCurrentProc eglMakeCurrentFn;
  eglPostSubBufferNVProc eglPostSubBufferNVFn;
  eglProgramCacheGetAttribANGLEProc eglProgramCacheGetAttribANGLEFn;
  eglProgramCachePopulateANGLEProc eglProgramCachePopulateANGLEFn;
  eglProgramCacheQueryANGLEProc eglProgramCacheQueryANGLEFn;
  eglProgramCacheResizeANGLEProc eglProgramCacheResizeANGLEFn;
  eglQueryAPIProc eglQueryAPIFn;
  eglQueryContextProc eglQueryContextFn;
  eglQueryStreamKHRProc eglQueryStreamKHRFn;
  eglQueryStreamu64KHRProc eglQueryStreamu64KHRFn;
  eglQueryStringProc eglQueryStringFn;
  eglQuerySurfaceProc eglQuerySurfaceFn;
  eglQuerySurfacePointerANGLEProc eglQuerySurfacePointerANGLEFn;
  eglReleaseTexImageProc eglReleaseTexImageFn;
  eglReleaseThreadProc eglReleaseThreadFn;
  eglStreamAttribKHRProc eglStreamAttribKHRFn;
  eglStreamConsumerAcquireKHRProc eglStreamConsumerAcquireKHRFn;
  eglStreamConsumerGLTextureExternalAttribsNVProc
      eglStreamConsumerGLTextureExternalAttribsNVFn;
  eglStreamConsumerGLTextureExternalKHRProc
      eglStreamConsumerGLTextureExternalKHRFn;
  eglStreamConsumerReleaseKHRProc eglStreamConsumerReleaseKHRFn;
  eglStreamPostD3DTextureNV12ANGLEProc eglStreamPostD3DTextureNV12ANGLEFn;
  eglSurfaceAttribProc eglSurfaceAttribFn;
  eglSwapBuffersProc eglSwapBuffersFn;
  eglSwapBuffersWithDamageKHRProc eglSwapBuffersWithDamageKHRFn;
  eglSwapIntervalProc eglSwapIntervalFn;
  eglTerminateProc eglTerminateFn;
  eglWaitClientProc eglWaitClientFn;
  eglWaitGLProc eglWaitGLFn;
  eglWaitNativeProc eglWaitNativeFn;
  eglWaitSyncKHRProc eglWaitSyncKHRFn;
};

class GL_EXPORT EGLApi {
 public:
  EGLApi();
  virtual ~EGLApi();

  virtual void SetDisabledExtensions(const std::string& disabled_extensions) {}

  virtual EGLBoolean eglBindAPIFn(EGLenum api) = 0;
  virtual EGLBoolean eglBindTexImageFn(EGLDisplay dpy,
                                       EGLSurface surface,
                                       EGLint buffer) = 0;
  virtual EGLBoolean eglChooseConfigFn(EGLDisplay dpy,
                                       const EGLint* attrib_list,
                                       EGLConfig* configs,
                                       EGLint config_size,
                                       EGLint* num_config) = 0;
  virtual EGLint eglClientWaitSyncKHRFn(EGLDisplay dpy,
                                        EGLSyncKHR sync,
                                        EGLint flags,
                                        EGLTimeKHR timeout) = 0;
  virtual EGLBoolean eglCopyBuffersFn(EGLDisplay dpy,
                                      EGLSurface surface,
                                      EGLNativePixmapType target) = 0;
  virtual EGLContext eglCreateContextFn(EGLDisplay dpy,
                                        EGLConfig config,
                                        EGLContext share_context,
                                        const EGLint* attrib_list) = 0;
  virtual EGLImageKHR eglCreateImageKHRFn(EGLDisplay dpy,
                                          EGLContext ctx,
                                          EGLenum target,
                                          EGLClientBuffer buffer,
                                          const EGLint* attrib_list) = 0;
  virtual EGLSurface eglCreatePbufferFromClientBufferFn(
      EGLDisplay dpy,
      EGLenum buftype,
      void* buffer,
      EGLConfig config,
      const EGLint* attrib_list) = 0;
  virtual EGLSurface eglCreatePbufferSurfaceFn(EGLDisplay dpy,
                                               EGLConfig config,
                                               const EGLint* attrib_list) = 0;
  virtual EGLSurface eglCreatePixmapSurfaceFn(EGLDisplay dpy,
                                              EGLConfig config,
                                              EGLNativePixmapType pixmap,
                                              const EGLint* attrib_list) = 0;
  virtual EGLStreamKHR eglCreateStreamKHRFn(EGLDisplay dpy,
                                            const EGLint* attrib_list) = 0;
  virtual EGLBoolean eglCreateStreamProducerD3DTextureNV12ANGLEFn(
      EGLDisplay dpy,
      EGLStreamKHR stream,
      EGLAttrib* attrib_list) = 0;
  virtual EGLSyncKHR eglCreateSyncKHRFn(EGLDisplay dpy,
                                        EGLenum type,
                                        const EGLint* attrib_list) = 0;
  virtual EGLSurface eglCreateWindowSurfaceFn(EGLDisplay dpy,
                                              EGLConfig config,
                                              EGLNativeWindowType win,
                                              const EGLint* attrib_list) = 0;
  virtual EGLBoolean eglDestroyContextFn(EGLDisplay dpy, EGLContext ctx) = 0;
  virtual EGLBoolean eglDestroyImageKHRFn(EGLDisplay dpy,
                                          EGLImageKHR image) = 0;
  virtual EGLBoolean eglDestroyStreamKHRFn(EGLDisplay dpy,
                                           EGLStreamKHR stream) = 0;
  virtual EGLBoolean eglDestroySurfaceFn(EGLDisplay dpy,
                                         EGLSurface surface) = 0;
  virtual EGLBoolean eglDestroySyncKHRFn(EGLDisplay dpy, EGLSyncKHR sync) = 0;
  virtual EGLBoolean eglGetConfigAttribFn(EGLDisplay dpy,
                                          EGLConfig config,
                                          EGLint attribute,
                                          EGLint* value) = 0;
  virtual EGLBoolean eglGetConfigsFn(EGLDisplay dpy,
                                     EGLConfig* configs,
                                     EGLint config_size,
                                     EGLint* num_config) = 0;
  virtual EGLContext eglGetCurrentContextFn(void) = 0;
  virtual EGLDisplay eglGetCurrentDisplayFn(void) = 0;
  virtual EGLSurface eglGetCurrentSurfaceFn(EGLint readdraw) = 0;
  virtual EGLDisplay eglGetDisplayFn(EGLNativeDisplayType display_id) = 0;
  virtual EGLint eglGetErrorFn(void) = 0;
  virtual EGLDisplay eglGetPlatformDisplayEXTFn(EGLenum platform,
                                                void* native_display,
                                                const EGLint* attrib_list) = 0;
  virtual __eglMustCastToProperFunctionPointerType eglGetProcAddressFn(
      const char* procname) = 0;
  virtual EGLBoolean eglGetSyncAttribKHRFn(EGLDisplay dpy,
                                           EGLSyncKHR sync,
                                           EGLint attribute,
                                           EGLint* value) = 0;
  virtual EGLBoolean eglGetSyncValuesCHROMIUMFn(EGLDisplay dpy,
                                                EGLSurface surface,
                                                EGLuint64CHROMIUM* ust,
                                                EGLuint64CHROMIUM* msc,
                                                EGLuint64CHROMIUM* sbc) = 0;
  virtual EGLBoolean eglImageFlushExternalEXTFn(
      EGLDisplay dpy,
      EGLImageKHR image,
      const EGLAttrib* attrib_list) = 0;
  virtual EGLBoolean eglInitializeFn(EGLDisplay dpy,
                                     EGLint* major,
                                     EGLint* minor) = 0;
  virtual EGLBoolean eglMakeCurrentFn(EGLDisplay dpy,
                                      EGLSurface draw,
                                      EGLSurface read,
                                      EGLContext ctx) = 0;
  virtual EGLBoolean eglPostSubBufferNVFn(EGLDisplay dpy,
                                          EGLSurface surface,
                                          EGLint x,
                                          EGLint y,
                                          EGLint width,
                                          EGLint height) = 0;
  virtual EGLint eglProgramCacheGetAttribANGLEFn(EGLDisplay dpy,
                                                 EGLenum attrib) = 0;
  virtual void eglProgramCachePopulateANGLEFn(EGLDisplay dpy,
                                              const void* key,
                                              EGLint keysize,
                                              const void* binary,
                                              EGLint binarysize) = 0;
  virtual void eglProgramCacheQueryANGLEFn(EGLDisplay dpy,
                                           EGLint index,
                                           void* key,
                                           EGLint* keysize,
                                           void* binary,
                                           EGLint* binarysize) = 0;
  virtual EGLint eglProgramCacheResizeANGLEFn(EGLDisplay dpy,
                                              EGLint limit,
                                              EGLenum mode) = 0;
  virtual EGLenum eglQueryAPIFn(void) = 0;
  virtual EGLBoolean eglQueryContextFn(EGLDisplay dpy,
                                       EGLContext ctx,
                                       EGLint attribute,
                                       EGLint* value) = 0;
  virtual EGLBoolean eglQueryStreamKHRFn(EGLDisplay dpy,
                                         EGLStreamKHR stream,
                                         EGLenum attribute,
                                         EGLint* value) = 0;
  virtual EGLBoolean eglQueryStreamu64KHRFn(EGLDisplay dpy,
                                            EGLStreamKHR stream,
                                            EGLenum attribute,
                                            EGLuint64KHR* value) = 0;
  virtual const char* eglQueryStringFn(EGLDisplay dpy, EGLint name) = 0;
  virtual EGLBoolean eglQuerySurfaceFn(EGLDisplay dpy,
                                       EGLSurface surface,
                                       EGLint attribute,
                                       EGLint* value) = 0;
  virtual EGLBoolean eglQuerySurfacePointerANGLEFn(EGLDisplay dpy,
                                                   EGLSurface surface,
                                                   EGLint attribute,
                                                   void** value) = 0;
  virtual EGLBoolean eglReleaseTexImageFn(EGLDisplay dpy,
                                          EGLSurface surface,
                                          EGLint buffer) = 0;
  virtual EGLBoolean eglReleaseThreadFn(void) = 0;
  virtual EGLBoolean eglStreamAttribKHRFn(EGLDisplay dpy,
                                          EGLStreamKHR stream,
                                          EGLenum attribute,
                                          EGLint value) = 0;
  virtual EGLBoolean eglStreamConsumerAcquireKHRFn(EGLDisplay dpy,
                                                   EGLStreamKHR stream) = 0;
  virtual EGLBoolean eglStreamConsumerGLTextureExternalAttribsNVFn(
      EGLDisplay dpy,
      EGLStreamKHR stream,
      EGLAttrib* attrib_list) = 0;
  virtual EGLBoolean eglStreamConsumerGLTextureExternalKHRFn(
      EGLDisplay dpy,
      EGLStreamKHR stream) = 0;
  virtual EGLBoolean eglStreamConsumerReleaseKHRFn(EGLDisplay dpy,
                                                   EGLStreamKHR stream) = 0;
  virtual EGLBoolean eglStreamPostD3DTextureNV12ANGLEFn(
      EGLDisplay dpy,
      EGLStreamKHR stream,
      void* texture,
      const EGLAttrib* attrib_list) = 0;
  virtual EGLBoolean eglSurfaceAttribFn(EGLDisplay dpy,
                                        EGLSurface surface,
                                        EGLint attribute,
                                        EGLint value) = 0;
  virtual EGLBoolean eglSwapBuffersFn(EGLDisplay dpy, EGLSurface surface) = 0;
  virtual EGLBoolean eglSwapBuffersWithDamageKHRFn(EGLDisplay dpy,
                                                   EGLSurface surface,
                                                   EGLint* rects,
                                                   EGLint n_rects) = 0;
  virtual EGLBoolean eglSwapIntervalFn(EGLDisplay dpy, EGLint interval) = 0;
  virtual EGLBoolean eglTerminateFn(EGLDisplay dpy) = 0;
  virtual EGLBoolean eglWaitClientFn(void) = 0;
  virtual EGLBoolean eglWaitGLFn(void) = 0;
  virtual EGLBoolean eglWaitNativeFn(EGLint engine) = 0;
  virtual EGLint eglWaitSyncKHRFn(EGLDisplay dpy,
                                  EGLSyncKHR sync,
                                  EGLint flags) = 0;
};

}  // namespace gl

#define eglBindAPI ::gl::g_current_egl_context->eglBindAPIFn
#define eglBindTexImage ::gl::g_current_egl_context->eglBindTexImageFn
#define eglChooseConfig ::gl::g_current_egl_context->eglChooseConfigFn
#define eglClientWaitSyncKHR ::gl::g_current_egl_context->eglClientWaitSyncKHRFn
#define eglCopyBuffers ::gl::g_current_egl_context->eglCopyBuffersFn
#define eglCreateContext ::gl::g_current_egl_context->eglCreateContextFn
#define eglCreateImageKHR ::gl::g_current_egl_context->eglCreateImageKHRFn
#define eglCreatePbufferFromClientBuffer \
  ::gl::g_current_egl_context->eglCreatePbufferFromClientBufferFn
#define eglCreatePbufferSurface \
  ::gl::g_current_egl_context->eglCreatePbufferSurfaceFn
#define eglCreatePixmapSurface \
  ::gl::g_current_egl_context->eglCreatePixmapSurfaceFn
#define eglCreateStreamKHR ::gl::g_current_egl_context->eglCreateStreamKHRFn
#define eglCreateStreamProducerD3DTextureNV12ANGLE \
  ::gl::g_current_egl_context->eglCreateStreamProducerD3DTextureNV12ANGLEFn
#define eglCreateSyncKHR ::gl::g_current_egl_context->eglCreateSyncKHRFn
#define eglCreateWindowSurface \
  ::gl::g_current_egl_context->eglCreateWindowSurfaceFn
#define eglDestroyContext ::gl::g_current_egl_context->eglDestroyContextFn
#define eglDestroyImageKHR ::gl::g_current_egl_context->eglDestroyImageKHRFn
#define eglDestroyStreamKHR ::gl::g_current_egl_context->eglDestroyStreamKHRFn
#define eglDestroySurface ::gl::g_current_egl_context->eglDestroySurfaceFn
#define eglDestroySyncKHR ::gl::g_current_egl_context->eglDestroySyncKHRFn
#define eglGetConfigAttrib ::gl::g_current_egl_context->eglGetConfigAttribFn
#define eglGetConfigs ::gl::g_current_egl_context->eglGetConfigsFn
#define eglGetCurrentContext ::gl::g_current_egl_context->eglGetCurrentContextFn
#define eglGetCurrentDisplay ::gl::g_current_egl_context->eglGetCurrentDisplayFn
#define eglGetCurrentSurface ::gl::g_current_egl_context->eglGetCurrentSurfaceFn
#define eglGetDisplay ::gl::g_current_egl_context->eglGetDisplayFn
#define eglGetError ::gl::g_current_egl_context->eglGetErrorFn
#define eglGetPlatformDisplayEXT \
  ::gl::g_current_egl_context->eglGetPlatformDisplayEXTFn
#define eglGetProcAddress ::gl::g_current_egl_context->eglGetProcAddressFn
#define eglGetSyncAttribKHR ::gl::g_current_egl_context->eglGetSyncAttribKHRFn
#define eglGetSyncValuesCHROMIUM \
  ::gl::g_current_egl_context->eglGetSyncValuesCHROMIUMFn
#define eglImageFlushExternalEXT \
  ::gl::g_current_egl_context->eglImageFlushExternalEXTFn
#define eglInitialize ::gl::g_current_egl_context->eglInitializeFn
#define eglMakeCurrent ::gl::g_current_egl_context->eglMakeCurrentFn
#define eglPostSubBufferNV ::gl::g_current_egl_context->eglPostSubBufferNVFn
#define eglProgramCacheGetAttribANGLE \
  ::gl::g_current_egl_context->eglProgramCacheGetAttribANGLEFn
#define eglProgramCachePopulateANGLE \
  ::gl::g_current_egl_context->eglProgramCachePopulateANGLEFn
#define eglProgramCacheQueryANGLE \
  ::gl::g_current_egl_context->eglProgramCacheQueryANGLEFn
#define eglProgramCacheResizeANGLE \
  ::gl::g_current_egl_context->eglProgramCacheResizeANGLEFn
#define eglQueryAPI ::gl::g_current_egl_context->eglQueryAPIFn
#define eglQueryContext ::gl::g_current_egl_context->eglQueryContextFn
#define eglQueryStreamKHR ::gl::g_current_egl_context->eglQueryStreamKHRFn
#define eglQueryStreamu64KHR ::gl::g_current_egl_context->eglQueryStreamu64KHRFn
#define eglQueryString ::gl::g_current_egl_context->eglQueryStringFn
#define eglQuerySurface ::gl::g_current_egl_context->eglQuerySurfaceFn
#define eglQuerySurfacePointerANGLE \
  ::gl::g_current_egl_context->eglQuerySurfacePointerANGLEFn
#define eglReleaseTexImage ::gl::g_current_egl_context->eglReleaseTexImageFn
#define eglReleaseThread ::gl::g_current_egl_context->eglReleaseThreadFn
#define eglStreamAttribKHR ::gl::g_current_egl_context->eglStreamAttribKHRFn
#define eglStreamConsumerAcquireKHR \
  ::gl::g_current_egl_context->eglStreamConsumerAcquireKHRFn
#define eglStreamConsumerGLTextureExternalAttribsNV \
  ::gl::g_current_egl_context->eglStreamConsumerGLTextureExternalAttribsNVFn
#define eglStreamConsumerGLTextureExternalKHR \
  ::gl::g_current_egl_context->eglStreamConsumerGLTextureExternalKHRFn
#define eglStreamConsumerReleaseKHR \
  ::gl::g_current_egl_context->eglStreamConsumerReleaseKHRFn
#define eglStreamPostD3DTextureNV12ANGLE \
  ::gl::g_current_egl_context->eglStreamPostD3DTextureNV12ANGLEFn
#define eglSurfaceAttrib ::gl::g_current_egl_context->eglSurfaceAttribFn
#define eglSwapBuffers ::gl::g_current_egl_context->eglSwapBuffersFn
#define eglSwapBuffersWithDamageKHR \
  ::gl::g_current_egl_context->eglSwapBuffersWithDamageKHRFn
#define eglSwapInterval ::gl::g_current_egl_context->eglSwapIntervalFn
#define eglTerminate ::gl::g_current_egl_context->eglTerminateFn
#define eglWaitClient ::gl::g_current_egl_context->eglWaitClientFn
#define eglWaitGL ::gl::g_current_egl_context->eglWaitGLFn
#define eglWaitNative ::gl::g_current_egl_context->eglWaitNativeFn
#define eglWaitSyncKHR ::gl::g_current_egl_context->eglWaitSyncKHRFn

#endif  //  UI_GL_GL_BINDINGS_AUTOGEN_EGL_H_

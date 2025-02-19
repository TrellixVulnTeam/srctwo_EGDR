// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gl/init/gl_factory.h"

#include "base/logging.h"
#include "base/trace_event/trace_event.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_context_egl.h"
#include "ui/gl/gl_context_osmesa.h"
#include "ui/gl/gl_context_stub.h"
#include "ui/gl/gl_context_wgl.h"
#include "ui/gl/gl_egl_api_implementation.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_share_group.h"
#include "ui/gl/gl_surface.h"
#include "ui/gl/gl_surface_egl.h"
#include "ui/gl/gl_surface_osmesa.h"
#include "ui/gl/gl_surface_osmesa_win.h"
#include "ui/gl/gl_surface_stub.h"
#include "ui/gl/gl_surface_wgl.h"
#include "ui/gl/gl_wgl_api_implementation.h"
#include "ui/gl/vsync_provider_win.h"

namespace gl {
namespace init {

std::vector<GLImplementation> GetAllowedGLImplementations() {
  std::vector<GLImplementation> impls;
  impls.push_back(kGLImplementationEGLGLES2);
  impls.push_back(kGLImplementationDesktopGL);
  impls.push_back(kGLImplementationOSMesaGL);
  impls.push_back(kGLImplementationSwiftShaderGL);
  return impls;
}

bool GetGLWindowSystemBindingInfo(GLWindowSystemBindingInfo* info) {
  switch (GetGLImplementation()) {
    case kGLImplementationDesktopGL:
      return GetGLWindowSystemBindingInfoWGL(info);
    case kGLImplementationEGLGLES2:
      return GetGLWindowSystemBindingInfoEGL(info);
    default:
      return false;
  }
}

scoped_refptr<GLContext> CreateGLContext(GLShareGroup* share_group,
                                         GLSurface* compatible_surface,
                                         const GLContextAttribs& attribs) {
  TRACE_EVENT0("gpu", "gl::init::CreateGLContext");
  switch (GetGLImplementation()) {
    case kGLImplementationOSMesaGL:
      return InitializeGLContext(new GLContextOSMesa(share_group),
                                 compatible_surface, attribs);
    case kGLImplementationSwiftShaderGL:
    case kGLImplementationEGLGLES2:
      return InitializeGLContext(new GLContextEGL(share_group),
                                 compatible_surface, attribs);
    case kGLImplementationDesktopGL:
      return InitializeGLContext(new GLContextWGL(share_group),
                                 compatible_surface, attribs);
    case kGLImplementationMockGL:
      return new GLContextStub(share_group);
    case kGLImplementationStubGL: {
      scoped_refptr<GLContextStub> stub_context =
          new GLContextStub(share_group);
      stub_context->SetUseStubApi(true);
      return stub_context;
    }
    default:
      NOTREACHED();
      return nullptr;
  }
}

scoped_refptr<GLSurface> CreateViewGLSurface(gfx::AcceleratedWidget window) {
  TRACE_EVENT0("gpu", "gl::init::CreateViewGLSurface");
  switch (GetGLImplementation()) {
    case kGLImplementationOSMesaGL:
      return InitializeGLSurface(new GLSurfaceOSMesaWin(window));
    case kGLImplementationSwiftShaderGL:
    case kGLImplementationEGLGLES2: {
      std::unique_ptr<gfx::VSyncProvider> sync_provider(
          new VSyncProviderWin(window));
      return CreateNativeViewGLSurfaceEGL(window, std::move(sync_provider));
    }
    case kGLImplementationDesktopGL:
      return InitializeGLSurface(new NativeViewGLSurfaceWGL(window));
    case kGLImplementationMockGL:
    case kGLImplementationStubGL:
      return new GLSurfaceStub;
    default:
      NOTREACHED();
      return nullptr;
  }
}

scoped_refptr<GLSurface> CreateNativeViewGLSurfaceEGL(
    gfx::AcceleratedWidget window,
    std::unique_ptr<gfx::VSyncProvider> sync_provider) {
  DCHECK_EQ(kGLImplementationEGLGLES2, GetGLImplementation());
  DCHECK(window != gfx::kNullAcceleratedWidget);

  return InitializeGLSurface(
      new NativeViewGLSurfaceEGL(window, std::move(sync_provider)));
}

scoped_refptr<GLSurface> CreateOffscreenGLSurfaceWithFormat(
    const gfx::Size& size, GLSurfaceFormat format) {
  TRACE_EVENT0("gpu", "gl::init::CreateOffscreenGLSurface");
  switch (GetGLImplementation()) {
    case kGLImplementationOSMesaGL:
      format.SetDefaultPixelLayout(GLSurfaceFormat::PIXEL_LAYOUT_RGBA);
      return InitializeGLSurfaceWithFormat(
          new GLSurfaceOSMesa(format, size), format);
    case kGLImplementationSwiftShaderGL:
    case kGLImplementationEGLGLES2:
      if (GLSurfaceEGL::IsEGLSurfacelessContextSupported() &&
          size.width() == 0 && size.height() == 0) {
        return InitializeGLSurfaceWithFormat(new SurfacelessEGL(size), format);
      } else {
        return InitializeGLSurfaceWithFormat(new PbufferGLSurfaceEGL(size),
                                             format);
      }
    case kGLImplementationDesktopGL:
      return InitializeGLSurfaceWithFormat(
          new PbufferGLSurfaceWGL(size), format);
    case kGLImplementationMockGL:
    case kGLImplementationStubGL:
      return new GLSurfaceStub;
    default:
      NOTREACHED();
      return nullptr;
  }
}

void SetDisabledExtensionsPlatform(const std::string& disabled_extensions) {
  GLImplementation implementation = GetGLImplementation();
  DCHECK_NE(kGLImplementationNone, implementation);
  switch (implementation) {
    case kGLImplementationDesktopGL:
      SetDisabledExtensionsWGL(disabled_extensions);
      break;
    case kGLImplementationEGLGLES2:
      SetDisabledExtensionsEGL(disabled_extensions);
      break;
    case kGLImplementationSwiftShaderGL:
    case kGLImplementationOSMesaGL:
    case kGLImplementationMockGL:
    case kGLImplementationStubGL:
      break;
    default:
      NOTREACHED();
  }
}

bool InitializeExtensionSettingsOneOffPlatform() {
  GLImplementation implementation = GetGLImplementation();
  DCHECK_NE(kGLImplementationNone, implementation);
  switch (implementation) {
    case kGLImplementationDesktopGL:
      return InitializeExtensionSettingsOneOffWGL();
    case kGLImplementationEGLGLES2:
      return InitializeExtensionSettingsOneOffEGL();
    case kGLImplementationSwiftShaderGL:
    case kGLImplementationOSMesaGL:
    case kGLImplementationMockGL:
    case kGLImplementationStubGL:
      return true;
    default:
      NOTREACHED();
      return false;
  }
}

}  // namespace init
}  // namespace gl

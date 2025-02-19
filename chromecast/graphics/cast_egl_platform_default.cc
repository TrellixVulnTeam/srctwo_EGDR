// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "chromecast/public/cast_egl_platform.h"
#include "chromecast/public/cast_egl_platform_shlib.h"

namespace chromecast {
namespace {

// Default/stub CastEglPlatform implementation so that we can link
// successfully.
class EglPlatformDefault : public CastEglPlatform {
 public:
  ~EglPlatformDefault() override {}
  const int* GetEGLSurfaceProperties(const int* desired) override {
    return desired;
  }
  bool InitializeHardware() override { return true; }
  void ShutdownHardware() override {}
  void* GetEglLibrary() override { return nullptr; }
  void* GetGles2Library() override { return nullptr; }
  GLGetProcAddressProc GetGLProcAddressProc() override { return nullptr; }
  NativeDisplayType CreateDisplayType(const Size& size) override {
    return nullptr;
  }
  void DestroyDisplayType(NativeDisplayType display_type) override {}
  NativeWindowType CreateWindow(NativeDisplayType display_type,
                                const Size& size) override {
    return nullptr;
  }
  void DestroyWindow(NativeWindowType window) override {}
  bool MultipleSurfaceUnsupported() override { return false; }
};

}  // namespace

CastEglPlatform* CastEglPlatformShlib::Create(
    const std::vector<std::string>& argv) {
  return new EglPlatformDefault();
}

}  // namespace chromecast

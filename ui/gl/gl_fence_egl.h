// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GL_GL_FENCE_EGL_H_
#define UI_GL_GL_FENCE_EGL_H_

#include "base/macros.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_export.h"
#include "ui/gl/gl_fence.h"

namespace gl {

class GL_EXPORT GLFenceEGL : public GLFence {
 public:
  static void SetIgnoreFailures();

  GLFenceEGL();
  ~GLFenceEGL() override;

  // GLFence implementation:
  bool HasCompleted() override;
  void ClientWait() override;
  void ServerWait() override;

  // EGL-specific wait-with-timeout implementation:
  EGLint ClientWaitWithTimeoutNanos(EGLTimeKHR timeout);

 private:
  EGLSyncKHR sync_;
  EGLDisplay display_;

  DISALLOW_COPY_AND_ASSIGN(GLFenceEGL);
};

}  // namespace gl

#endif  // UI_GL_GL_FENCE_EGL_H_

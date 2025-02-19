// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_GPU_MOCK_SURFACE_TEXTURE_GL_OWNER_H_
#define MEDIA_GPU_MOCK_SURFACE_TEXTURE_GL_OWNER_H_

#include "media/gpu/surface_texture_gl_owner.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_surface.h"

namespace media {

// This is a mock with a small amount of fake functionality too.
class MockSurfaceTextureGLOwner : public SurfaceTextureGLOwner {
 public:
  MockSurfaceTextureGLOwner(GLuint fake_texture_id,
                            gl::GLContext* fake_context,
                            gl::GLSurface* fake_surface);

  MOCK_CONST_METHOD0(GetTextureId, GLuint());
  MOCK_CONST_METHOD0(GetContext, gl::GLContext*());
  MOCK_CONST_METHOD0(GetSurface, gl::GLSurface*());
  MOCK_CONST_METHOD0(CreateJavaSurface, gl::ScopedJavaSurface());
  MOCK_METHOD0(UpdateTexImage, void());
  MOCK_METHOD1(GetTransformMatrix, void(float mtx[16]));
  MOCK_METHOD0(ReleaseBackBuffers, void());
  MOCK_METHOD0(SetReleaseTimeToNow, void());
  MOCK_METHOD0(IgnorePendingRelease, void());
  MOCK_METHOD0(IsExpectingFrameAvailable, bool());
  MOCK_METHOD0(WaitForFrameAvailable, void());

  // Fake implementations that the mocks will call by default.
  void FakeSetReleaseTimeToNow() { expecting_frame_available = true; }
  void FakeIgnorePendingRelease() { expecting_frame_available = false; }
  bool FakeIsExpectingFrameAvailable() { return expecting_frame_available; }
  void FakeWaitForFrameAvailable() { expecting_frame_available = false; }

  GLuint fake_texture_id;
  gl::GLContext* fake_context;
  gl::GLSurface* fake_surface;
  bool expecting_frame_available;

 protected:
  ~MockSurfaceTextureGLOwner();
};

}  // namespace media

#endif  // MEDIA_GPU_MOCK_SURFACE_TEXTURE_GL_OWNER_H_

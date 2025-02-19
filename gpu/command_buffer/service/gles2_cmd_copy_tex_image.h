// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_COMMAND_BUFFER_SERVICE_GLES2_CMD_COPY_TEX_IMAGE_H_
#define GPU_COMMAND_BUFFER_SERVICE_GLES2_CMD_COPY_TEX_IMAGE_H_

#include <array>

#include "base/containers/hash_tables.h"
#include "base/macros.h"
#include "gpu/command_buffer/service/feature_info.h"
#include "gpu/command_buffer/service/gl_utils.h"
#include "gpu/gpu_export.h"

namespace gpu {
namespace gles2 {

class GLES2Decoder;

// This class encapsulates the resources required to implement the
// glCopyTexImage and glCopyTexSubImage commands.  These commands somtimes
// require a blit.
class GPU_EXPORT CopyTexImageResourceManager {
 public:
  explicit CopyTexImageResourceManager(const gles2::FeatureInfo* feature_info);
  ~CopyTexImageResourceManager();

  void Initialize(const gles2::GLES2Decoder* decoder);
  void Destroy();

  void DoCopyTexImage2DToLUMACompatibilityTexture(
      const gles2::GLES2Decoder* decoder,
      GLuint dest_texture,
      GLenum dest_texture_target,
      GLenum dest_target,
      GLenum luma_format,
      GLenum luma_type,
      GLint level,
      GLenum internal_format,
      GLint x,
      GLint y,
      GLsizei width,
      GLsizei height,
      GLuint source_framebuffer,
      GLenum source_framebuffer_internal_format);

  void DoCopyTexSubImageToLUMACompatibilityTexture(
      const gles2::GLES2Decoder* decoder,
      GLuint dest_texture,
      GLenum dest_texture_target,
      GLenum dest_target,
      GLenum luma_format,
      GLenum luma_type,
      GLint level,
      GLint xoffset,
      GLint yoffset,
      GLint zoffset,
      GLint x,
      GLint y,
      GLsizei width,
      GLsizei height,
      GLuint source_framebuffer,
      GLenum source_framebuffer_internal_format);

  static bool CopyTexImageRequiresBlit(const gles2::FeatureInfo* feature_info,
                                       GLenum dest_texture_format);

 private:
  scoped_refptr<const gles2::FeatureInfo> feature_info_;

  bool initialized_ = false;

  GLuint blit_program_ = 0;

  std::array<GLuint, 2> scratch_textures_ = {{0, 0}};
  GLuint scratch_fbo_ = 0;

  GLuint vao_ = 0;

  DISALLOW_COPY_AND_ASSIGN(CopyTexImageResourceManager);
};

}  // namespace gles2
}  // namespace gpu

#endif  // GPU_COMMAND_BUFFER_SERVICE_GLES2_CMD_COPY_TEX_IMAGE_H_

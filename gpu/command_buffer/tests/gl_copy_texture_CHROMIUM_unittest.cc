// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2extchromium.h>
#include <GLES3/gl3.h>
#include <stddef.h>
#include <stdint.h>

#include "gpu/command_buffer/tests/gl_manager.h"
#include "gpu/command_buffer/tests/gl_test_utils.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/gl/gl_version_info.h"

namespace gpu {

namespace {

enum CopyType { TexImage, TexSubImage };
const CopyType kCopyTypes[] = {
    TexImage,
    TexSubImage,
};

struct FormatType {
  GLenum internal_format;
  GLenum format;
  GLenum type;
};

static const char* kSimpleVertexShaderES2 =
    "attribute vec2 a_position;\n"
    "varying vec2 v_texCoord;\n"
    "void main() {\n"
    "  gl_Position = vec4(a_position.x, a_position.y, 0.0, 1.0);\n"
    "  v_texCoord = (a_position + vec2(1.0, 1.0)) * 0.5;\n"
    "}\n";

static const char* kSimpleVertexShaderES3 =
    "#version 300 es\n"
    "in vec2 a_position;\n"
    "out vec2 v_texCoord;\n"
    "void main() {\n"
    "  gl_Position = vec4(a_position.x, a_position.y, 0.0, 1.0);\n"
    "  v_texCoord = (a_position + vec2(1.0, 1.0)) * 0.5;\n"
    "}\n";

std::string GetFragmentShaderSource(GLenum target, GLenum format, bool is_es3) {
  std::string source;
  if (is_es3) {
    source +=
        "#version 300 es\n"
        "#define VARYING in\n"
        "#define FRAGCOLOR frag_color\n"
        "#define TextureLookup texture\n";
  } else {
    source +=
        "#define VARYING varying\n"
        "#define FRAGCOLOR gl_FragColor\n";
    if (target == GL_TEXTURE_CUBE_MAP) {
      source += "#define TextureLookup textureCube\n";
    } else {
      source += "#define TextureLookup texture2D\n";
    }
  }
  source += "precision mediump float;\n";

  if (gles2::GLES2Util::IsSignedIntegerFormat(format)) {
    if (target == GL_TEXTURE_CUBE_MAP) {
      source += "#define SamplerType isamplerCube\n";
    } else {
      source += "#define SamplerType isampler2D\n";
    }
    source += "#define TextureType ivec4\n";
    source += "#define ScaleValue 255.0\n";
  } else if (gles2::GLES2Util::IsUnsignedIntegerFormat(format)) {
    if (target == GL_TEXTURE_CUBE_MAP) {
      source += "#define SamplerType usamplerCube\n";
    } else {
      source += "#define SamplerType usampler2D\n";
    }
    source += "#define TextureType uvec4\n";
    source += "#define ScaleValue 255.0\n";
  } else {
    if (target == GL_TEXTURE_CUBE_MAP) {
      source += "#define SamplerType samplerCube\n";
    } else {
      source += "#define SamplerType sampler2D\n";
    }
    source += "#define TextureType vec4\n";
    source += "#define ScaleValue 1.0\n";
  }

  if (is_es3)
    source += "out vec4 frag_color;\n";

  if (target == GL_TEXTURE_CUBE_MAP)
    source += "uniform highp int u_face;\n";

  source +=
      "uniform mediump SamplerType u_texture;\n"
      "VARYING vec2 v_texCoord;\n"
      "void main() {\n";

  if (target == GL_TEXTURE_CUBE_MAP) {
    source +=
        "  vec3 texCube;\n"
        // Transform [0, 1] to [-1, 1].
        "  vec2 texCoord = (v_texCoord * 2.0) - 1.0;\n"
        // Transform 2d tex coord to each face of TEXTURE_CUBE_MAP coord.
        // TEXTURE_CUBE_MAP_POSITIVE_X
        "  if (u_face == 34069) {\n"
        "    texCube = vec3(1.0, -texCoord.y, -texCoord.x);\n"
        // TEXTURE_CUBE_MAP_NEGATIVE_X
        "  } else if (u_face == 34070) {\n"
        "    texCube = vec3(-1.0, -texCoord.y, texCoord.x);\n"
        // TEXTURE_CUBE_MAP_POSITIVE_Y
        "  } else if (u_face == 34071) {\n"
        "    texCube = vec3(texCoord.x, 1.0, texCoord.y);\n"
        // TEXTURE_CUBE_MAP_NEGATIVE_Y
        "  } else if (u_face == 34072) {\n"
        "    texCube = vec3(texCoord.x, -1.0, -texCoord.y);\n"
        // TEXTURE_CUBE_MAP_POSITIVE_Z
        "  } else if (u_face == 34073) {\n"
        "    texCube = vec3(texCoord.x, -texCoord.y, 1.0);\n"
        // TEXTURE_CUBE_MAP_NEGATIVE_Z
        "  } else if (u_face == 34074) {\n"
        "    texCube = vec3(-texCoord.x, -texCoord.y, -1.0);\n"
        "  }\n"
        "  TextureType color = TextureLookup(u_texture, texCube);\n";
  } else {
    source += "  TextureType color = TextureLookup(u_texture, v_texCoord);\n";
  }

  source +=
      "  FRAGCOLOR = vec4(color) / ScaleValue;\n"
      "}\n";
  return source;
}

void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a, uint8_t* color) {
  color[0] = r;
  color[1] = g;
  color[2] = b;
  color[3] = a;
}

void getExpectedColor(GLenum src_internal_format,
                      GLenum dest_internal_format,
                      uint8_t* color,
                      uint8_t* expected_color,
                      uint8_t* mask) {
  uint8_t adjusted_color[4];
  switch (src_internal_format) {
    case GL_ALPHA:
      setColor(0, 0, 0, color[0], adjusted_color);
      break;
    case GL_R8:
    case GL_R16_EXT:
      setColor(color[0], 0, 0, 255, adjusted_color);
      break;
    case GL_LUMINANCE:
      setColor(color[0], color[0], color[0], 255, adjusted_color);
      break;
    case GL_LUMINANCE_ALPHA:
      setColor(color[0], color[0], color[0], color[1], adjusted_color);
      break;
    case GL_RGB:
    case GL_RGB8:
    case GL_RGB_YCBCR_420V_CHROMIUM:
    case GL_RGB_YCBCR_422_CHROMIUM:
      setColor(color[0], color[1], color[2], 255, adjusted_color);
      break;
    case GL_RGBA:
    case GL_RGBA8:
      setColor(color[0], color[1], color[2], color[3], adjusted_color);
      break;
    case GL_BGRA_EXT:
    case GL_BGRA8_EXT:
      setColor(color[2], color[1], color[0], color[3], adjusted_color);
      break;
    default:
      NOTREACHED();
      break;
  }

  switch (dest_internal_format) {
    case GL_ALPHA:
      setColor(0, 0, 0, adjusted_color[3], expected_color);
      setColor(0, 0, 0, 1, mask);
      break;
    case GL_R8:
    case GL_R16F:
    case GL_R32F:
    case GL_R8UI:
      setColor(adjusted_color[0], 0, 0, 0, expected_color);
      setColor(1, 0, 0, 0, mask);
      break;
    case GL_LUMINANCE:
      setColor(adjusted_color[0], 0, 0, 0, expected_color);
      setColor(1, 0, 0, 0, mask);
      break;
    case GL_LUMINANCE_ALPHA:
      setColor(adjusted_color[0], 0, 0, adjusted_color[3], expected_color);
      setColor(1, 0, 0, 1, mask);
      break;
    case GL_RG8:
    case GL_RG16F:
    case GL_RG32F:
    case GL_RG8UI:
      setColor(adjusted_color[0], adjusted_color[1], 0, 0, expected_color);
      setColor(1, 1, 0, 0, mask);
      break;
    case GL_RGB:
    case GL_RGB8:
    case GL_SRGB_EXT:
    case GL_SRGB8:
    case GL_RGB565:
    case GL_R11F_G11F_B10F:
    case GL_RGB9_E5:
    case GL_RGB16F:
    case GL_RGB32F:
    case GL_RGB8UI:
      setColor(adjusted_color[0], adjusted_color[1], adjusted_color[2], 0,
               expected_color);
      setColor(1, 1, 1, 0, mask);
      break;
    case GL_RGBA:
    case GL_RGBA8:
    case GL_BGRA_EXT:
    case GL_BGRA8_EXT:
    case GL_SRGB_ALPHA_EXT:
    case GL_SRGB8_ALPHA8:
    case GL_RGBA4:
    case GL_RGBA16F:
    case GL_RGBA32F:
    case GL_RGBA8UI:
      setColor(adjusted_color[0], adjusted_color[1], adjusted_color[2],
               adjusted_color[3], expected_color);
      setColor(1, 1, 1, 1, mask);
      break;
    case GL_RGB5_A1:
      setColor(adjusted_color[0], adjusted_color[1], adjusted_color[2],
               (adjusted_color[3] >> 7) ? 0xFF : 0x0, expected_color);
      // TODO(qiankun.miao@intel.com): On some Windows platforms, the alpha
      // channel of expected color is the source alpha value other than 255.
      // This should be wrong. Skip the alpha channel check and revisit this in
      // future.
      setColor(1, 1, 1, 0, mask);
      break;
    default:
      NOTREACHED();
      break;
  }
}

std::unique_ptr<uint8_t[]> getTextureDataAndExpectedRGBA(
    FormatType src_format_type,
    FormatType dest_format_type,
    GLsizei width,
    GLsizei height,
    uint8_t* expected_color,
    uint8_t* expected_mask) {
  const int src_channel_count = gles2::GLES2Util::ElementsPerGroup(
      src_format_type.format, src_format_type.type);
  uint8_t color[4] = {1u, 63u, 127u, 255u};
  getExpectedColor(src_format_type.internal_format,
                   dest_format_type.internal_format, color, expected_color,
                   expected_mask);
  if (src_format_type.type == GL_UNSIGNED_BYTE) {
    std::unique_ptr<uint8_t[]> pixels(
        new uint8_t[width * height * src_channel_count]);
    for (int i = 0; i < width * height * src_channel_count;
         i += src_channel_count) {
      for (int j = 0; j < src_channel_count; ++j)
        pixels[i + j] = color[j];
    }
    return pixels;
  } else if (src_format_type.type == GL_UNSIGNED_SHORT) {
    uint16_t color_16bit[4] = {1u << 8, 63u << 8, 127u << 8, 255u << 8};
    std::unique_ptr<uint8_t[]> data(
        new uint8_t[width * height * src_channel_count * sizeof(uint16_t)]);
    uint16_t* pixels = reinterpret_cast<uint16_t*>(data.get());
    int16_t flip_sign = -1;
    for (int i = 0; i < width * height * src_channel_count;
         i += src_channel_count) {
      for (int j = 0; j < src_channel_count; ++j) {
        // Introduce an offset to the value to check. Expected value should be
        // the same as without the offset.
        flip_sign *= -1;
        pixels[i + j] =
            color_16bit[j] + flip_sign * (0x7F * (i + j)) / (width * height);
      }
    }
    return data;
  }
  NOTREACHED();
  return nullptr;
}

}  // namespace

// A collection of tests that exercise the GL_CHROMIUM_copy_texture extension.
class GLCopyTextureCHROMIUMTest
    : public testing::Test,
      public ::testing::WithParamInterface<CopyType> {
 protected:
  void CreateAndBindDestinationTextureAndFBO(GLenum target) {
    glGenTextures(2, textures_);
    glBindTexture(target, textures_[1]);

    // Some drivers (NVidia/SGX) require texture settings to be a certain way or
    // they won't report FRAMEBUFFER_COMPLETE.
    glTexParameterf(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glGenFramebuffers(1, &framebuffer_id_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target,
                           textures_[1], 0);
  }

  void SetUp() override {
    GLManager::Options options;
    options.size = gfx::Size(64, 64);
    gl_.Initialize(options);

    width_ = 8;
    height_ = 8;
  }

  void TearDown() override { gl_.Destroy(); }

  void CreateBackingForTexture(GLenum target, GLsizei width, GLsizei height) {
    if (target == GL_TEXTURE_RECTANGLE_ARB) {
      std::unique_ptr<gfx::GpuMemoryBuffer> buffer(gl_.CreateGpuMemoryBuffer(
          gfx::Size(width, height), gfx::BufferFormat::RGBA_8888));
      GLuint image_id = glCreateImageCHROMIUM(buffer->AsClientBuffer(), width,
                                              height, GL_RGBA);
      glBindTexImage2DCHROMIUM(target, image_id);
    } else {
      glTexImage2D(target, 0, GL_RGBA, width, height, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, nullptr);
    }
  }

  GLuint CreateDrawingTexture(GLenum target, GLsizei width, GLsizei height) {
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(target, texture);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CreateBackingForTexture(target, width, height);
    return texture;
  }

  GLuint CreateDrawingFBO(GLenum target, GLuint texture) {
    GLuint framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target,
                           texture, 0);
    return framebuffer;
  }

  GLenum ExtractFormatFrom(GLenum internalformat) {
    switch (internalformat) {
      case GL_RGBA8_OES:
        return GL_RGBA;
      case GL_RGB8_OES:
        return GL_RGB;
      case GL_BGRA8_EXT:
        return GL_BGRA_EXT;
      default:
        NOTREACHED();
        return GL_NONE;
    }
  }

  void RunCopyTexture(GLenum dest_target,
                      CopyType copy_type,
                      FormatType src_format_type,
                      GLint source_level,
                      FormatType dest_format_type,
                      GLint dest_level,
                      bool is_es3) {
    uint8_t expected_color[4];
    uint8_t mask[4];
    std::unique_ptr<uint8_t[]> pixels =
        getTextureDataAndExpectedRGBA(src_format_type, dest_format_type, width_,
                                      height_, expected_color, mask);
    GLenum source_target = GL_TEXTURE_2D;
    glGenTextures(2, textures_);
    glBindTexture(source_target, textures_[0]);
    glTexParameteri(source_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(source_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#if defined(OS_MACOSX)
    // TODO(qiankun.miao@intel.com): Remove this workaround for Mac OSX, once
    // integer texture rendering bug is fixed on Mac OSX: crbug.com/679639.
    glTexImage2D(source_target, 0, src_format_type.internal_format,
                 width_ << source_level, height_ << source_level, 0,
                 src_format_type.format, src_format_type.type, nullptr);
#endif
    glTexImage2D(source_target, source_level, src_format_type.internal_format,
                 width_, height_, 0, src_format_type.format,
                 src_format_type.type, pixels.get());
    EXPECT_TRUE(glGetError() == GL_NO_ERROR);
    GLenum dest_binding_target =
        gles2::GLES2Util::GLFaceTargetToTextureTarget(dest_target);
    glBindTexture(dest_binding_target, textures_[1]);
    glTexParameterf(dest_binding_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(dest_binding_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(dest_binding_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // This hack makes dest texture complete in ES2 and ES3 context
    // respectively. With this, sampling from the dest texture is correct.
    if (is_es3) {
      glTexParameteri(dest_binding_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(dest_binding_target, GL_TEXTURE_BASE_LEVEL, dest_level);
      // For cube map textures, make the texture cube complete.
      if (dest_binding_target == GL_TEXTURE_CUBE_MAP) {
        for (int i = 0; i < 6; ++i) {
          GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
          glTexImage2D(face, dest_level, dest_format_type.internal_format,
                       width_, height_, 0, dest_format_type.format,
                       dest_format_type.type, nullptr);
        }
      }
#if defined(OS_MACOSX)
      // TODO(qiankun.miao@intel.com): Remove this workaround for Mac OSX, once
      // framebuffer complete bug is fixed on Mac OSX: crbug.com/678526.
      glTexImage2D(dest_target, 0, dest_format_type.internal_format,
                   width_ << dest_level, height_ << dest_level, 0,
                   dest_format_type.format, dest_format_type.type, nullptr);
#endif
    } else {
      glTexParameteri(dest_binding_target, GL_TEXTURE_MIN_FILTER,
                      GL_NEAREST_MIPMAP_NEAREST);
      // For cube map textures, make the texture cube complete.
      if (dest_binding_target == GL_TEXTURE_CUBE_MAP) {
        for (int i = 0; i < 6; ++i) {
          GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
          glTexImage2D(face, 0, dest_format_type.internal_format,
                       width_ << dest_level, height_ << dest_level, 0,
                       dest_format_type.format, dest_format_type.type, nullptr);
        }
      } else {
        glTexImage2D(dest_target, 0, dest_format_type.internal_format,
                     width_ << dest_level, height_ << dest_level, 0,
                     dest_format_type.format, dest_format_type.type, nullptr);
      }
      glGenerateMipmap(dest_binding_target);
    }
    EXPECT_TRUE(glGetError() == GL_NO_ERROR);

    if (copy_type == TexImage) {
      glCopyTextureCHROMIUM(textures_[0], source_level, dest_target,
                            textures_[1], dest_level,
                            dest_format_type.internal_format,
                            dest_format_type.type, false, false, false);
    } else {
      glBindTexture(dest_binding_target, textures_[1]);
      glTexImage2D(dest_target, dest_level, dest_format_type.internal_format,
                   width_, height_, 0, dest_format_type.format,
                   dest_format_type.type, nullptr);

      glCopySubTextureCHROMIUM(textures_[0], source_level, dest_target,
                               textures_[1], dest_level, 0, 0, 0, 0, width_,
                               height_, false, false, false);
    }
    EXPECT_TRUE(glGetError() == GL_NO_ERROR);

    // Draw destination texture to a fbo with a TEXTURE_2D texture attachment
    // in RGBA format.
    GLuint texture = CreateDrawingTexture(GL_TEXTURE_2D, width_, height_);
    GLuint framebuffer = CreateDrawingFBO(GL_TEXTURE_2D, texture);
    EXPECT_EQ(static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE),
              glCheckFramebufferStatus(GL_FRAMEBUFFER));
    glViewport(0, 0, width_, height_);

    glBindTexture(dest_binding_target, textures_[1]);
    std::string fragment_shader_source = GetFragmentShaderSource(
        dest_binding_target, dest_format_type.internal_format, is_es3);
    GLTestHelper::DrawTextureQuad(
        dest_target, is_es3 ? kSimpleVertexShaderES3 : kSimpleVertexShaderES2,
        fragment_shader_source.c_str(), "a_position", "u_texture", "u_face");
    EXPECT_TRUE(GL_NO_ERROR == glGetError());

    uint8_t tolerance = dest_format_type.internal_format == GL_RGBA4 ? 20 : 7;
    EXPECT_TRUE(GLTestHelper::CheckPixels(0, 0, width_, height_, tolerance,
                                          expected_color, mask))
        << " dest_target : " << gles2::GLES2Util::GetStringEnum(dest_target)
        << " src_internal_format: "
        << gles2::GLES2Util::GetStringEnum(src_format_type.internal_format)
        << " source_level: " << source_level << " dest_internal_format: "
        << gles2::GLES2Util::GetStringEnum(dest_format_type.internal_format)
        << " dest_level: " << dest_level;

    glDeleteTextures(1, &texture);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(2, textures_);
  }

  GLManager gl_;
  GLuint textures_[2];
  GLsizei width_;
  GLsizei height_;
  GLuint framebuffer_id_;
};

class GLCopyTextureCHROMIUMES3Test : public GLCopyTextureCHROMIUMTest {
 protected:
  void SetUp() override {
    GLManager::Options options;
    options.context_type = gles2::CONTEXT_TYPE_OPENGLES3;
    options.size = gfx::Size(64, 64);
    GpuDriverBugWorkarounds workarounds;
#if defined(OS_MACOSX)
    // Sampling of seamless integer cube map texture has bug on Intel GEN7 gpus
    // on Mac OSX, see crbug.com/658930.
    workarounds.disable_texture_cube_map_seamless = true;
#endif
    gl_.InitializeWithWorkarounds(options, workarounds);

    width_ = 8;
    height_ = 8;
  }

  // If a driver isn't capable of supporting ES3 context, creating
  // ContextGroup will fail. Just skip the test.
  bool ShouldSkipTest() const {
    return (!gl_.decoder() || !gl_.decoder()->GetContextGroup());
  }

  // If EXT_color_buffer_float isn't available, float format isn't supported.
  bool ShouldSkipFloatFormat() const {
    DCHECK(!ShouldSkipTest());
    return !gl_.decoder()->GetFeatureInfo()->ext_color_buffer_float_available();
  }

  bool ShouldSkipBGRA() const {
    DCHECK(!ShouldSkipTest());
    return !gl_.decoder()
                ->GetFeatureInfo()
                ->feature_flags()
                .ext_texture_format_bgra8888;
  }

  bool ShouldSkipSRGBEXT() const {
    DCHECK(!ShouldSkipTest());
    return !gl_.decoder()->GetFeatureInfo()->feature_flags().ext_srgb;
  }

  bool ShouldSkipNorm16() const {
    DCHECK(!ShouldSkipTest());
#if (defined(OS_MACOSX) || defined(OS_WIN) || defined(OS_LINUX)) && \
    (defined(ARCH_CPU_X86) || defined(ARCH_CPU_X86_64))
    // Make sure it's tested; it is safe to assume that the flag is always true
    // on desktop.
    EXPECT_TRUE(
        gl_.decoder()->GetFeatureInfo()->feature_flags().ext_texture_norm16);
#endif
    return !gl_.decoder()->GetFeatureInfo()->feature_flags().ext_texture_norm16;
  }
};

INSTANTIATE_TEST_CASE_P(CopyType,
                        GLCopyTextureCHROMIUMTest,
                        ::testing::ValuesIn(kCopyTypes));

INSTANTIATE_TEST_CASE_P(CopyType,
                        GLCopyTextureCHROMIUMES3Test,
                        ::testing::ValuesIn(kCopyTypes));

// Test to ensure that the basic functionality of the extension works.
TEST_P(GLCopyTextureCHROMIUMTest, Basic) {
  CopyType copy_type = GetParam();
  uint8_t pixels[1 * 4] = {255u, 0u, 0u, 255u};

  CreateAndBindDestinationTextureAndFBO(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textures_[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               pixels);

  if (copy_type == TexImage) {
    glCopyTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0,
                          GL_RGBA, GL_UNSIGNED_BYTE, false, false, false);
  } else {
    glBindTexture(GL_TEXTURE_2D, textures_[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);

    glCopySubTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0, 0,
                             0, 0, 0, 1, 1, false, false, false);
  }
  EXPECT_TRUE(glGetError() == GL_NO_ERROR);

  // Check the FB is still bound.
  GLint value = 0;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &value);
  GLuint fb_id = value;
  EXPECT_EQ(framebuffer_id_, fb_id);

  // Check that FB is complete.
  EXPECT_EQ(static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE),
            glCheckFramebufferStatus(GL_FRAMEBUFFER));

  GLTestHelper::CheckPixels(0, 0, 1, 1, 0, pixels, nullptr);
  EXPECT_TRUE(GL_NO_ERROR == glGetError());
}

TEST_P(GLCopyTextureCHROMIUMES3Test, FormatCombinations) {
  if (ShouldSkipTest())
    return;
  if (gl_.gpu_preferences().use_passthrough_cmd_decoder) {
    // TODO(geofflang): anglebug.com/1932
    LOG(INFO)
        << "Passthrough command decoder expected failure. Skipping test...";
    return;
  }
  CopyType copy_type = GetParam();

  FormatType src_format_types[] = {
      {GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE},
      {GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE},
      {GL_RGB, GL_RGB, GL_UNSIGNED_BYTE},
      {GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE},
      {GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE},
      {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE},
      {GL_BGRA_EXT, GL_BGRA_EXT, GL_UNSIGNED_BYTE},
      {GL_BGRA8_EXT, GL_BGRA_EXT, GL_UNSIGNED_BYTE},
      {GL_R16_EXT, GL_RED, GL_UNSIGNED_SHORT},
  };

  FormatType dest_format_types[] = {
      // TODO(qiankun.miao@intel.com): ALPHA and LUMINANCE formats have bug on
      // GL core profile. See crbug.com/577144. Enable these formats after
      // using workaround in gles2_cmd_copy_tex_image.cc.
      // {GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE},
      // {GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE},
      // {GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE},

      {GL_RGB, GL_RGB, GL_UNSIGNED_BYTE},
      {GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE},
      {GL_SRGB_EXT, GL_SRGB_EXT, GL_UNSIGNED_BYTE},
      {GL_SRGB_ALPHA_EXT, GL_SRGB_ALPHA_EXT, GL_UNSIGNED_BYTE},
      {GL_BGRA_EXT, GL_BGRA_EXT, GL_UNSIGNED_BYTE},
      {GL_BGRA8_EXT, GL_BGRA_EXT, GL_UNSIGNED_BYTE},
      {GL_R8, GL_RED, GL_UNSIGNED_BYTE},
      {GL_R16F, GL_RED, GL_HALF_FLOAT},
      {GL_R16F, GL_RED, GL_FLOAT},
      {GL_R32F, GL_RED, GL_FLOAT},
      {GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE},
      {GL_RG8, GL_RG, GL_UNSIGNED_BYTE},
      {GL_RG16F, GL_RG, GL_HALF_FLOAT},
      {GL_RG16F, GL_RG, GL_FLOAT},
      {GL_RG32F, GL_RG, GL_FLOAT},
      {GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_BYTE},
      {GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE},
      {GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE},
      {GL_RGB565, GL_RGB, GL_UNSIGNED_BYTE},
      {GL_R11F_G11F_B10F, GL_RGB, GL_FLOAT},
      {GL_RGB9_E5, GL_RGB, GL_HALF_FLOAT},
      {GL_RGB9_E5, GL_RGB, GL_FLOAT},
      {GL_RGB16F, GL_RGB, GL_HALF_FLOAT},
      {GL_RGB16F, GL_RGB, GL_FLOAT},
      {GL_RGB32F, GL_RGB, GL_FLOAT},
      {GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE},
      {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE},
      {GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE},
      {GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_BYTE},
      {GL_RGBA4, GL_RGBA, GL_UNSIGNED_BYTE},
      {GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT},
      {GL_RGBA16F, GL_RGBA, GL_FLOAT},
      {GL_RGBA32F, GL_RGBA, GL_FLOAT},
      {GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE},
  };

  for (auto src_format_type : src_format_types) {
    for (auto dest_format_type : dest_format_types) {
      if ((src_format_type.internal_format == GL_BGRA_EXT ||
           src_format_type.internal_format == GL_BGRA8_EXT ||
           dest_format_type.internal_format == GL_BGRA_EXT ||
           dest_format_type.internal_format == GL_BGRA8_EXT) &&
          ShouldSkipBGRA()) {
        continue;
      }
      if (gles2::GLES2Util::IsFloatFormat(dest_format_type.internal_format) &&
          ShouldSkipFloatFormat())
        continue;
      if ((dest_format_type.internal_format == GL_SRGB_EXT ||
           dest_format_type.internal_format == GL_SRGB_ALPHA_EXT) &&
          ShouldSkipSRGBEXT())
        continue;
      if (src_format_type.internal_format == GL_R16_EXT && ShouldSkipNorm16())
        continue;

      RunCopyTexture(GL_TEXTURE_2D, copy_type, src_format_type, 0,
                     dest_format_type, 0, true);
    }
  }
}

TEST_P(GLCopyTextureCHROMIUMTest, ImmutableTexture) {
  if (!GLTestHelper::HasExtension("GL_EXT_texture_storage")) {
    LOG(INFO) << "GL_EXT_texture_storage not supported. Skipping test...";
    return;
  }
  CopyType copy_type = GetParam();
  GLenum src_internal_formats[] = {GL_RGB8_OES, GL_RGBA8_OES, GL_BGRA8_EXT};
  GLenum dest_internal_formats[] = {GL_RGB8_OES, GL_RGBA8_OES, GL_BGRA8_EXT};

  uint8_t pixels[1 * 4] = {255u, 0u, 255u, 255u};

  for (auto src_internal_format : src_internal_formats) {
    for (auto dest_internal_format : dest_internal_formats) {
      CreateAndBindDestinationTextureAndFBO(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, textures_[0]);
      glTexStorage2DEXT(GL_TEXTURE_2D, 1, src_internal_format, 1, 1);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1,
                      ExtractFormatFrom(src_internal_format), GL_UNSIGNED_BYTE,
                      pixels);

      glBindTexture(GL_TEXTURE_2D, textures_[1]);
      glTexStorage2DEXT(GL_TEXTURE_2D, 1, dest_internal_format, 1, 1);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_2D, textures_[1], 0);
      EXPECT_TRUE(glGetError() == GL_NO_ERROR);

      if (copy_type == TexImage) {
        glCopyTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0,
                              ExtractFormatFrom(dest_internal_format),
                              GL_UNSIGNED_BYTE, false, false, false);
        EXPECT_TRUE(glGetError() == GL_INVALID_OPERATION);
      } else {
        glCopySubTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1],
                                 0, 0, 0, 0, 0, 1, 1, false, false, false);
        EXPECT_TRUE(glGetError() == GL_NO_ERROR);

        // Check the FB is still bound.
        GLint value = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &value);
        GLuint fb_id = value;
        EXPECT_EQ(framebuffer_id_, fb_id);

        // Check that FB is complete.
        EXPECT_EQ(static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE),
                  glCheckFramebufferStatus(GL_FRAMEBUFFER));

        GLTestHelper::CheckPixels(0, 0, 1, 1, 0, pixels, nullptr);
        EXPECT_TRUE(GL_NO_ERROR == glGetError());
      }
      glDeleteTextures(2, textures_);
      glDeleteFramebuffers(1, &framebuffer_id_);
    }
  }
}

TEST_P(GLCopyTextureCHROMIUMTest, InternalFormat) {
  CopyType copy_type = GetParam();
  GLint src_formats[] = {GL_ALPHA,     GL_RGB,             GL_RGBA,
                         GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_BGRA_EXT};
  GLint dest_formats[] = {GL_RGB, GL_RGBA, GL_BGRA_EXT};

  for (size_t src_index = 0; src_index < arraysize(src_formats); src_index++) {
    for (size_t dest_index = 0; dest_index < arraysize(dest_formats);
         dest_index++) {
      CreateAndBindDestinationTextureAndFBO(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, textures_[0]);
      glTexImage2D(GL_TEXTURE_2D, 0, src_formats[src_index], 1, 1, 0,
                   src_formats[src_index], GL_UNSIGNED_BYTE, nullptr);
      EXPECT_TRUE(GL_NO_ERROR == glGetError());

      if (copy_type == TexImage) {
        glCopyTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0,
                              dest_formats[dest_index], GL_UNSIGNED_BYTE, false,
                              false, false);
      } else {
        glBindTexture(GL_TEXTURE_2D, textures_[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, dest_formats[dest_index], 1, 1, 0,
                     dest_formats[dest_index], GL_UNSIGNED_BYTE, nullptr);
        EXPECT_TRUE(GL_NO_ERROR == glGetError());

        glCopySubTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1],
                                 0, 0, 0, 0, 0, 1, 1, false, false, false);
      }

      EXPECT_TRUE(GL_NO_ERROR == glGetError()) << "src_index:" << src_index
                                               << " dest_index:" << dest_index;
      glDeleteTextures(2, textures_);
      glDeleteFramebuffers(1, &framebuffer_id_);
    }
  }
}

TEST_P(GLCopyTextureCHROMIUMTest, InternalFormatRGBFloat) {
  if (!GLTestHelper::HasExtension("GL_CHROMIUM_color_buffer_float_rgb")) {
    LOG(INFO)
        << "GL_CHROMIUM_color_buffer_float_rgb not supported. Skipping test...";
    return;
  }
  // TODO(qiankun.miao@intel.com): since RunCopyTexture requires dest texture to
  // be texture complete, skip this test if float texture is not color
  // filterable. We should remove this limitation when we find a way doesn't
  // require dest texture to be texture complete in RunCopyTexture.
  if (!gl_.decoder()
           ->GetFeatureInfo()
           ->feature_flags()
           .enable_texture_float_linear) {
    LOG(INFO) << "RGB32F texture is not filterable. Skipping test...";
    return;
  }
  CopyType copy_type = GetParam();
  FormatType src_format_type = {GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE};
  FormatType dest_format_type = {GL_RGB32F, GL_RGB, GL_FLOAT};

  RunCopyTexture(GL_TEXTURE_2D, copy_type, src_format_type, 0, dest_format_type,
                 0, false);
}

TEST_P(GLCopyTextureCHROMIUMTest, InternalFormatRGBAFloat) {
  if (!GLTestHelper::HasExtension("GL_CHROMIUM_color_buffer_float_rgba")) {
    LOG(INFO) << "GL_CHROMIUM_color_buffer_float_rgba not supported. Skipping "
                 "test...";
    return;
  }
  // TODO(qiankun.miao@intel.com): since RunCopyTexture requires dest texture to
  // be texture complete, skip this test if float texture is not color
  // filterable. We should remove this limitation when we find a way doesn't
  // require dest texture to be texture complete in RunCopyTexture.
  if (!gl_.decoder()
           ->GetFeatureInfo()
           ->feature_flags()
           .enable_texture_float_linear) {
    LOG(INFO) << "RGBA32F texture is not filterable. Skipping test...";
    return;
  }
  CopyType copy_type = GetParam();
  FormatType src_format_type = {GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE};
  FormatType dest_format_type = {GL_RGBA32F, GL_RGBA, GL_FLOAT};

  RunCopyTexture(GL_TEXTURE_2D, copy_type, src_format_type, 0, dest_format_type,
                 0, false);
}

TEST_P(GLCopyTextureCHROMIUMTest, InternalFormatNotSupported) {
  CopyType copy_type = GetParam();
  CreateAndBindDestinationTextureAndFBO(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textures_[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               nullptr);
  EXPECT_TRUE(GL_NO_ERROR == glGetError());

  // Check unsupported format reports error.
  GLint unsupported_dest_formats[] = {GL_RED, GL_RG};
  for (size_t dest_index = 0; dest_index < arraysize(unsupported_dest_formats);
       dest_index++) {
    if (copy_type == TexImage) {
      glCopyTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0,
                            unsupported_dest_formats[dest_index],
                            GL_UNSIGNED_BYTE, false, false, false);
    } else {
      glBindTexture(GL_TEXTURE_2D, textures_[1]);
      glTexImage2D(GL_TEXTURE_2D, 0, unsupported_dest_formats[dest_index], 1, 1,
                   0, unsupported_dest_formats[dest_index], GL_UNSIGNED_BYTE,
                   nullptr);
      glCopySubTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0,
                               0, 0, 0, 0, 1, 1, false, false, false);
    }
    EXPECT_TRUE(GL_INVALID_OPERATION == glGetError())
        << "dest_index:" << dest_index;
  }
  glDeleteTextures(2, textures_);
  glDeleteFramebuffers(1, &framebuffer_id_);
}

TEST_F(GLCopyTextureCHROMIUMTest, InternalFormatTypeCombinationNotSupported) {
  CreateAndBindDestinationTextureAndFBO(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textures_[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               nullptr);
  EXPECT_TRUE(GL_NO_ERROR == glGetError());

  // Check unsupported internal_format/type combination reports error.
  struct FormatType { GLenum format, type; };
  FormatType unsupported_format_types[] = {
    {GL_RGB, GL_UNSIGNED_SHORT_4_4_4_4},
    {GL_RGB, GL_UNSIGNED_SHORT_5_5_5_1},
    {GL_RGBA, GL_UNSIGNED_SHORT_5_6_5},
  };
  for (size_t dest_index = 0; dest_index < arraysize(unsupported_format_types);
       dest_index++) {
    glCopyTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0,
                          unsupported_format_types[dest_index].format,
                          unsupported_format_types[dest_index].type, false,
                          false, false);
    EXPECT_TRUE(GL_INVALID_OPERATION == glGetError())
        << "dest_index:" << dest_index;
  }
  glDeleteTextures(2, textures_);
  glDeleteFramebuffers(1, &framebuffer_id_);
}

TEST_P(GLCopyTextureCHROMIUMTest, CopyTextureLevel) {
  CopyType copy_type = GetParam();

  // Copy from RGB source texture to dest texture.
  FormatType src_format_type = {GL_RGB, GL_RGB, GL_UNSIGNED_BYTE};
  FormatType dest_format_types[] = {
      {GL_RGB, GL_RGB, GL_UNSIGNED_BYTE},
      {GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE},
  };
  // Source level must be 0 in ES2 context.
  GLint source_level = 0;

  for (GLint dest_level = 0; dest_level < 3; dest_level++) {
    for (auto dest_format_type : dest_format_types) {
      RunCopyTexture(GL_TEXTURE_2D, copy_type, src_format_type, source_level,
                     dest_format_type, dest_level, false);
    }
  }
}

TEST_P(GLCopyTextureCHROMIUMES3Test, CopyTextureLevel) {
  if (ShouldSkipTest())
    return;
  if (gl_.gpu_preferences().use_passthrough_cmd_decoder) {
    // TODO(geofflang): anglebug.com/1932
    LOG(INFO)
        << "Passthrough command decoder expected failure. Skipping test...";
    return;
  }
  CopyType copy_type = GetParam();

  // Copy from RGBA source texture to dest texture.
  FormatType src_format_type = {GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE};
  FormatType dest_format_types[] = {
      {GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE},
      {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE},
      {GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE},
  };

  for (GLint source_level = 0; source_level < 3; source_level++) {
    for (GLint dest_level = 0; dest_level < 3; dest_level++) {
      for (auto dest_format_type : dest_format_types) {
#if defined(OS_WIN) || defined(OS_ANDROID)
        // TODO(qiankun.miao@intel.com): source_level > 0 or dest_level > 0
        // isn't available due to renderinig bug for non-zero base level in
        // NVIDIA Windows: crbug.com/679639 and Android: crbug.com/680460.
        if (dest_level > 0)
          continue;
#endif
        RunCopyTexture(GL_TEXTURE_2D, copy_type, src_format_type, source_level,
                       dest_format_type, dest_level, true);
      }
    }
  }
}

TEST_P(GLCopyTextureCHROMIUMTest, CopyTextureCubeMap) {
  CopyType copy_type = GetParam();

  GLenum dest_targets[] = {
      GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
      GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
      GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};
  FormatType src_format_type = {GL_RGB, GL_RGB, GL_UNSIGNED_BYTE};
  FormatType dest_format_types[] = {
      {GL_RGB, GL_RGB, GL_UNSIGNED_BYTE}, {GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE},
  };
  GLint source_level = 0;
  GLint dest_level = 0;

  for (auto dest_format_type : dest_format_types) {
    for (auto dest_target : dest_targets) {
      RunCopyTexture(dest_target, copy_type, src_format_type, source_level,
                     dest_format_type, dest_level, false);
    }
  }
}

TEST_P(GLCopyTextureCHROMIUMES3Test, CopyTextureCubeMap) {
  if (ShouldSkipTest())
    return;
  CopyType copy_type = GetParam();

  GLenum dest_targets[] = {
      GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
      GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
      GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};
  FormatType src_format_type = {GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE};
  FormatType dest_format_types[] = {
      {GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE},
      {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE},
      {GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE},
  };
  GLint source_level = 0;
  GLint dest_level = 0;

  for (auto dest_format_type : dest_format_types) {
    for (auto dest_target : dest_targets) {
      RunCopyTexture(dest_target, copy_type, src_format_type, source_level,
                     dest_format_type, dest_level, true);
    }
  }
}

// Test to ensure that the destination texture is redefined if the properties
// are different.
TEST_F(GLCopyTextureCHROMIUMTest, RedefineDestinationTexture) {
  uint8_t pixels[4 * 4] = {255u, 0u, 0u, 255u, 255u, 0u, 0u, 255u,
                           255u, 0u, 0u, 255u, 255u, 0u, 0u, 255u};

  CreateAndBindDestinationTextureAndFBO(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textures_[0]);
  glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  glBindTexture(GL_TEXTURE_2D, textures_[1]);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_BGRA_EXT,
               1,
               1,
               0,
               GL_BGRA_EXT,
               GL_UNSIGNED_BYTE,
               pixels);
  EXPECT_TRUE(GL_NO_ERROR == glGetError());

  // GL_INVALID_OPERATION due to "intrinsic format" != "internal format".
  glTexSubImage2D(
      GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  EXPECT_TRUE(GL_INVALID_OPERATION == glGetError());
  // GL_INVALID_VALUE due to bad dimensions.
  glTexSubImage2D(
      GL_TEXTURE_2D, 0, 1, 1, 1, 1, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);
  EXPECT_TRUE(GL_INVALID_VALUE == glGetError());

  // If the dest texture has different properties, glCopyTextureCHROMIUM()
  // redefines them.
  glCopyTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0,
                        GL_RGBA, GL_UNSIGNED_BYTE, false, false, false);
  EXPECT_TRUE(GL_NO_ERROR == glGetError());

  // glTexSubImage2D() succeeds because textures_[1] is redefined into 2x2
  // dimension and GL_RGBA format.
  glBindTexture(GL_TEXTURE_2D, textures_[1]);
  glTexSubImage2D(
      GL_TEXTURE_2D, 0, 1, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  EXPECT_TRUE(GL_NO_ERROR == glGetError());

  // Check the FB is still bound.
  GLint value = 0;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &value);
  GLuint fb_id = value;
  EXPECT_EQ(framebuffer_id_, fb_id);

  // Check that FB is complete.
  EXPECT_EQ(static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE),
            glCheckFramebufferStatus(GL_FRAMEBUFFER));

  GLTestHelper::CheckPixels(1, 1, 1, 1, 0, &pixels[12], nullptr);

  glDeleteTextures(2, textures_);
  glDeleteFramebuffers(1, &framebuffer_id_);

  EXPECT_TRUE(GL_NO_ERROR == glGetError());
}

namespace {

void glEnableDisable(GLint param, GLboolean value) {
  if (value)
    glEnable(param);
  else
    glDisable(param);
}

}  // unnamed namespace

// Validate that some basic GL state is not touched upon execution of
// the extension.
TEST_P(GLCopyTextureCHROMIUMTest, BasicStatePreservation) {
  CopyType copy_type = GetParam();
  uint8_t pixels[1 * 4] = {255u, 0u, 0u, 255u};

  CreateAndBindDestinationTextureAndFBO(GL_TEXTURE_2D);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glBindTexture(GL_TEXTURE_2D, textures_[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               pixels);

  if (copy_type == TexSubImage) {
    glBindTexture(GL_TEXTURE_2D, textures_[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
  }

  GLboolean reference_settings[2] = { GL_TRUE, GL_FALSE };
  for (int x = 0; x < 2; ++x) {
    GLboolean setting = reference_settings[x];
    glEnableDisable(GL_DEPTH_TEST, setting);
    glEnableDisable(GL_SCISSOR_TEST, setting);
    glEnableDisable(GL_STENCIL_TEST, setting);
    glEnableDisable(GL_CULL_FACE, setting);
    glEnableDisable(GL_BLEND, setting);
    glColorMask(setting, setting, setting, setting);
    glDepthMask(setting);

    glActiveTexture(GL_TEXTURE1 + x);

    if (copy_type == TexImage) {
      glCopyTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0,
                            GL_RGBA, GL_UNSIGNED_BYTE, false, false, false);
    } else {
      glCopySubTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0,
                               0, 0, 0, 0, 1, 1, false, false, false);
    }
    EXPECT_TRUE(GL_NO_ERROR == glGetError());

    EXPECT_EQ(setting, glIsEnabled(GL_DEPTH_TEST));
    EXPECT_EQ(setting, glIsEnabled(GL_SCISSOR_TEST));
    EXPECT_EQ(setting, glIsEnabled(GL_STENCIL_TEST));
    EXPECT_EQ(setting, glIsEnabled(GL_CULL_FACE));
    EXPECT_EQ(setting, glIsEnabled(GL_BLEND));

    GLboolean bool_array[4] = { GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE };
    glGetBooleanv(GL_DEPTH_WRITEMASK, bool_array);
    EXPECT_EQ(setting, bool_array[0]);

    bool_array[0] = GL_FALSE;
    glGetBooleanv(GL_COLOR_WRITEMASK, bool_array);
    EXPECT_EQ(setting, bool_array[0]);
    EXPECT_EQ(setting, bool_array[1]);
    EXPECT_EQ(setting, bool_array[2]);
    EXPECT_EQ(setting, bool_array[3]);

    GLint active_texture = 0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);
    EXPECT_EQ(GL_TEXTURE1 + x, active_texture);
  }

  glDeleteTextures(2, textures_);
  glDeleteFramebuffers(1, &framebuffer_id_);

  EXPECT_TRUE(GL_NO_ERROR == glGetError());
};

// Verify that invocation of the extension does not modify the bound
// texture state.
TEST_P(GLCopyTextureCHROMIUMTest, TextureStatePreserved) {
  CopyType copy_type = GetParam();
  // Setup the texture used for the extension invocation.
  uint8_t pixels[1 * 4] = {255u, 0u, 0u, 255u};
  CreateAndBindDestinationTextureAndFBO(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textures_[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               pixels);

  if (copy_type == TexSubImage) {
    glBindTexture(GL_TEXTURE_2D, textures_[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
  }

  GLuint texture_ids[2];
  glGenTextures(2, texture_ids);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_ids[0]);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture_ids[1]);

  if (copy_type == TexImage) {
    glCopyTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0,
                          GL_RGBA, GL_UNSIGNED_BYTE, false, false, false);
  } else {
    glCopySubTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0, 0,
                             0, 0, 0, 1, 1, false, false, false);
  }
  EXPECT_TRUE(GL_NO_ERROR == glGetError());

  GLint active_texture = 0;
  glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);
  EXPECT_EQ(GL_TEXTURE1, active_texture);

  GLint bound_texture = 0;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &bound_texture);
  EXPECT_EQ(texture_ids[1], static_cast<GLuint>(bound_texture));
  glBindTexture(GL_TEXTURE_2D, 0);

  bound_texture = 0;
  glActiveTexture(GL_TEXTURE0);
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &bound_texture);
  EXPECT_EQ(texture_ids[0], static_cast<GLuint>(bound_texture));
  glBindTexture(GL_TEXTURE_2D, 0);

  glDeleteTextures(2, texture_ids);
  glDeleteTextures(2, textures_);
  glDeleteFramebuffers(1, &framebuffer_id_);

  EXPECT_TRUE(GL_NO_ERROR == glGetError());
}

// Verify that invocation of the extension does not perturb the currently
// bound FBO state.
TEST_P(GLCopyTextureCHROMIUMTest, FBOStatePreserved) {
  CopyType copy_type = GetParam();
  // Setup the texture used for the extension invocation.
  uint8_t pixels[1 * 4] = {255u, 0u, 0u, 255u};
  CreateAndBindDestinationTextureAndFBO(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textures_[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               pixels);

  if (copy_type == TexSubImage) {
    glBindTexture(GL_TEXTURE_2D, textures_[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
  }

  GLuint texture_id;
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               0);

  GLuint renderbuffer_id;
  glGenRenderbuffers(1, &renderbuffer_id);
  glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_id);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 1, 1);

  GLuint framebuffer_id;
  glGenFramebuffers(1, &framebuffer_id);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         texture_id, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, renderbuffer_id);
  EXPECT_TRUE(
      GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_FRAMEBUFFER));

  // Test that we can write to the bound framebuffer
  uint8_t expected_color[4] = {255u, 255u, 0, 255u};
  glClearColor(1.0, 1.0, 0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  GLTestHelper::CheckPixels(0, 0, 1, 1, 0, expected_color, nullptr);

  if (copy_type == TexImage) {
    glCopyTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0,
                          GL_RGBA, GL_UNSIGNED_BYTE, false, false, false);
  } else {
    glCopySubTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0, 0,
                             0, 0, 0, 1, 1, false, false, false);
  }
  EXPECT_TRUE(GL_NO_ERROR == glGetError());

  EXPECT_TRUE(glIsFramebuffer(framebuffer_id));

  // Ensure that reading from the framebuffer produces correct pixels.
  GLTestHelper::CheckPixels(0, 0, 1, 1, 0, expected_color, nullptr);

  uint8_t expected_color2[4] = {255u, 0, 255u, 255u};
  glClearColor(1.0, 0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  GLTestHelper::CheckPixels(0, 0, 1, 1, 0, expected_color2, nullptr);

  GLint bound_fbo = 0;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &bound_fbo);
  EXPECT_EQ(framebuffer_id, static_cast<GLuint>(bound_fbo));

  GLint fbo_params = 0;
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                        GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
                                        &fbo_params);
  EXPECT_EQ(GL_TEXTURE, fbo_params);

  fbo_params = 0;
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                        GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                        &fbo_params);
  EXPECT_EQ(texture_id, static_cast<GLuint>(fbo_params));

  fbo_params = 0;
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                        GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
                                        &fbo_params);
  EXPECT_EQ(GL_RENDERBUFFER, fbo_params);

  fbo_params = 0;
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                        GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                        &fbo_params);
  EXPECT_EQ(renderbuffer_id, static_cast<GLuint>(fbo_params));

  glDeleteRenderbuffers(1, &renderbuffer_id);
  glDeleteTextures(1, &texture_id);
  glDeleteFramebuffers(1, &framebuffer_id);
  glDeleteTextures(2, textures_);
  glDeleteFramebuffers(1, &framebuffer_id_);

  EXPECT_TRUE(GL_NO_ERROR == glGetError());
}

TEST_P(GLCopyTextureCHROMIUMTest, ProgramStatePreservation) {
  CopyType copy_type = GetParam();
  CreateAndBindDestinationTextureAndFBO(GL_TEXTURE_2D);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  GLManager gl2;
  GLManager::Options options;
  options.size = gfx::Size(16, 16);
  options.share_group_manager = &gl_;
  gl2.Initialize(options);
  gl_.MakeCurrent();

  static const char* v_shader_str =
      "attribute vec4 g_Position;\n"
      "void main()\n"
      "{\n"
      "   gl_Position = g_Position;\n"
      "}\n";
  static const char* f_shader_str =
      "precision mediump float;\n"
      "void main()\n"
      "{\n"
      "  gl_FragColor = vec4(0,1,0,1);\n"
      "}\n";

  GLuint program = GLTestHelper::LoadProgram(v_shader_str, f_shader_str);
  glUseProgram(program);
  GLuint position_loc = glGetAttribLocation(program, "g_Position");
  glFlush();

  // Delete program from other context.
  gl2.MakeCurrent();
  glDeleteProgram(program);
  EXPECT_TRUE(GL_NO_ERROR == glGetError());
  glFlush();

  // Program should still be usable on this context.
  gl_.MakeCurrent();

  GLTestHelper::SetupUnitQuad(position_loc);

  // test using program before
  uint8_t expected[] = {
      0, 255, 0, 255,
  };
  uint8_t zero[] = {
      0, 0, 0, 0,
  };
  glClear(GL_COLOR_BUFFER_BIT);
  EXPECT_TRUE(GLTestHelper::CheckPixels(0, 0, 1, 1, 0, zero, nullptr));
  glDrawArrays(GL_TRIANGLES, 0, 6);
  EXPECT_TRUE(GLTestHelper::CheckPixels(0, 0, 1, 1, 0, expected, nullptr));

  // Call copyTextureCHROMIUM
  uint8_t pixels[1 * 4] = {255u, 0u, 0u, 255u};
  glBindTexture(GL_TEXTURE_2D, textures_[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               pixels);
  if (copy_type == TexImage) {
    glCopyTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0,
                          GL_RGBA, GL_UNSIGNED_BYTE, false, false, false);
  } else {
    glBindTexture(GL_TEXTURE_2D, textures_[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glCopySubTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0, 0,
                             0, 0, 0, 1, 1, false, false, false);
  }

  // test using program after
  glClear(GL_COLOR_BUFFER_BIT);
  EXPECT_TRUE(GLTestHelper::CheckPixels(0, 0, 1, 1, 0, zero, nullptr));
  glDrawArrays(GL_TRIANGLES, 0, 6);
  EXPECT_TRUE(GLTestHelper::CheckPixels(0, 0, 1, 1, 0, expected, nullptr));

  glDeleteTextures(2, textures_);
  glDeleteFramebuffers(1, &framebuffer_id_);

  EXPECT_TRUE(GL_NO_ERROR == glGetError());

  gl2.MakeCurrent();
  gl2.Destroy();
  gl_.MakeCurrent();
}

// Test that glCopyTextureCHROMIUM doesn't leak uninitialized textures.
TEST_P(GLCopyTextureCHROMIUMTest, UninitializedSource) {
  CopyType copy_type = GetParam();
  const GLsizei kWidth = 64, kHeight = 64;
  CreateAndBindDestinationTextureAndFBO(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textures_[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, nullptr);

  if (copy_type == TexImage) {
    glCopyTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0,
                          GL_RGBA, GL_UNSIGNED_BYTE, false, false, false);
  } else {
    glBindTexture(GL_TEXTURE_2D, textures_[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glCopySubTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0, 0,
                             0, 0, 0, kWidth, kHeight, false, false, false);
  }
  EXPECT_TRUE(GL_NO_ERROR == glGetError());

  uint8_t pixels[kHeight][kWidth][4] = {{{1}}};
  glReadPixels(0, 0, kWidth, kHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  for (int x = 0; x < kWidth; ++x) {
    for (int y = 0; y < kHeight; ++y) {
      EXPECT_EQ(0, pixels[y][x][0]);
      EXPECT_EQ(0, pixels[y][x][1]);
      EXPECT_EQ(0, pixels[y][x][2]);
      EXPECT_EQ(0, pixels[y][x][3]);
    }
  }

  glDeleteTextures(2, textures_);
  glDeleteFramebuffers(1, &framebuffer_id_);

  EXPECT_TRUE(GL_NO_ERROR == glGetError());
}

TEST_F(GLCopyTextureCHROMIUMTest, CopySubTextureDimension) {
  CreateAndBindDestinationTextureAndFBO(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textures_[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               nullptr);

  glBindTexture(GL_TEXTURE_2D, textures_[1]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 3, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               nullptr);

  glCopySubTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0, 1,
                           1, 0, 0, 1, 1, false, false, false);
  EXPECT_TRUE(GL_NO_ERROR == glGetError());

  // xoffset < 0
  glCopySubTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0, -1,
                           1, 0, 0, 1, 1, false, false, false);
  EXPECT_TRUE(glGetError() == GL_INVALID_VALUE);

  // x < 0
  glCopySubTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0, 1,
                           1, -1, 0, 1, 1, false, false, false);
  EXPECT_TRUE(glGetError() == GL_INVALID_VALUE);

  // xoffset + width > dest_width
  glCopySubTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0, 2,
                           2, 0, 0, 2, 2, false, false, false);
  EXPECT_TRUE(glGetError() == GL_INVALID_VALUE);

  // x + width > source_width
  glCopySubTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0, 0,
                           0, 1, 1, 2, 2, false, false, false);
  EXPECT_TRUE(glGetError() == GL_INVALID_VALUE);

  glDeleteTextures(2, textures_);
  glDeleteFramebuffers(1, &framebuffer_id_);
}

TEST_F(GLCopyTextureCHROMIUMTest, CopyTextureInvalidTextureIds) {
  CreateAndBindDestinationTextureAndFBO(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textures_[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               nullptr);

  glBindTexture(GL_TEXTURE_2D, textures_[1]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 3, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               nullptr);

  glCopyTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, 99993, 0, GL_RGBA,
                        GL_UNSIGNED_BYTE, false, false, false);
  EXPECT_TRUE(GL_INVALID_VALUE == glGetError());

  glCopyTextureCHROMIUM(99994, 0, GL_TEXTURE_2D, textures_[1], 0, GL_RGBA,
                        GL_UNSIGNED_BYTE, false, false, false);
  EXPECT_TRUE(GL_INVALID_VALUE == glGetError());

  glCopyTextureCHROMIUM(99995, 0, GL_TEXTURE_2D, 99996, 0, GL_RGBA,
                        GL_UNSIGNED_BYTE, false, false, false);
  EXPECT_TRUE(GL_INVALID_VALUE == glGetError());

  glCopyTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0,
                        GL_RGBA, GL_UNSIGNED_BYTE, false, false, false);
  EXPECT_TRUE(GL_NO_ERROR == glGetError());

  glDeleteTextures(2, textures_);
  glDeleteFramebuffers(1, &framebuffer_id_);
}

TEST_F(GLCopyTextureCHROMIUMTest, CopySubTextureInvalidTextureIds) {
  CreateAndBindDestinationTextureAndFBO(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textures_[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               nullptr);

  glBindTexture(GL_TEXTURE_2D, textures_[1]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 3, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               nullptr);

  glCopySubTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, 99993, 0, 1, 1, 0, 0,
                           1, 1, false, false, false);
  EXPECT_TRUE(GL_INVALID_VALUE == glGetError());

  glCopySubTextureCHROMIUM(99994, 0, GL_TEXTURE_2D, textures_[1], 0, 1, 1, 0, 0,
                           1, 1, false, false, false);
  EXPECT_TRUE(GL_INVALID_VALUE == glGetError());

  glCopySubTextureCHROMIUM(99995, 0, GL_TEXTURE_2D, 99996, 0, 1, 1, 0, 0, 1, 1,
                           false, false, false);
  EXPECT_TRUE(GL_INVALID_VALUE == glGetError());

  glCopySubTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0, 1,
                           1, 0, 0, 1, 1, false, false, false);
  EXPECT_TRUE(GL_NO_ERROR == glGetError());

  glDeleteTextures(2, textures_);
  glDeleteFramebuffers(1, &framebuffer_id_);
}

TEST_F(GLCopyTextureCHROMIUMTest, CopySubTextureOffset) {
  uint8_t rgba_pixels[4 * 4] = {255u, 0u, 0u,   255u, 0u, 255u, 0u, 255u,
                                0u,   0u, 255u, 255u, 0u, 0u,   0u, 255u};
  CreateAndBindDestinationTextureAndFBO(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textures_[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               rgba_pixels);

  uint8_t transparent_pixels[4 * 4] = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                                       0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
  glBindTexture(GL_TEXTURE_2D, textures_[1]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               transparent_pixels);

  glCopySubTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0, 1,
                           1, 0, 0, 1, 1, false, false, false);
  EXPECT_TRUE(glGetError() == GL_NO_ERROR);
  glCopySubTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0, 1,
                           0, 1, 0, 1, 1, false, false, false);
  EXPECT_TRUE(glGetError() == GL_NO_ERROR);
  glCopySubTextureCHROMIUM(textures_[0], 0, GL_TEXTURE_2D, textures_[1], 0, 0,
                           1, 0, 1, 1, 1, false, false, false);
  EXPECT_TRUE(glGetError() == GL_NO_ERROR);

  // Check the FB is still bound.
  GLint value = 0;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &value);
  GLuint fb_id = value;
  EXPECT_EQ(framebuffer_id_, fb_id);

  // Check that FB is complete.
  EXPECT_EQ(static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE),
            glCheckFramebufferStatus(GL_FRAMEBUFFER));

  uint8_t transparent[1 * 4] = {0u, 0u, 0u, 0u};
  uint8_t red[1 * 4] = {255u, 0u, 0u, 255u};
  uint8_t green[1 * 4] = {0u, 255u, 0u, 255u};
  uint8_t blue[1 * 4] = {0u, 0u, 255u, 255u};
  GLTestHelper::CheckPixels(0, 0, 1, 1, 0, transparent, nullptr);
  GLTestHelper::CheckPixels(1, 1, 1, 1, 0, red, nullptr);
  GLTestHelper::CheckPixels(1, 0, 1, 1, 0, green, nullptr);
  GLTestHelper::CheckPixels(0, 1, 1, 1, 0, blue, nullptr);
  EXPECT_TRUE(GL_NO_ERROR == glGetError());

  glDeleteTextures(2, textures_);
  glDeleteFramebuffers(1, &framebuffer_id_);
}

TEST_F(GLCopyTextureCHROMIUMTest, CopyTextureBetweenTexture2DAndRectangleArb) {
  if (!GLTestHelper::HasExtension("GL_ARB_texture_rectangle")) {
    LOG(INFO) <<
        "GL_ARB_texture_rectangle not supported. Skipping test...";
    return;
  }

  GLenum src_targets[] = {GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_2D};
  GLenum dest_targets[] = {GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_2D};
  GLsizei src_width = 30;
  GLsizei src_height = 14;
  GLsizei dest_width = 15;
  GLsizei dest_height = 13;
  GLsizei copy_region_x = 1;
  GLsizei copy_region_y = 1;
  GLsizei copy_region_width = 5;
  GLsizei copy_region_height = 3;
  uint8_t red[1 * 4] = {255u, 0u, 0u, 255u};
  uint8_t blue[1 * 4] = {0u, 0u, 255u, 255u};
  uint8_t green[1 * 4] = {0u, 255u, 0, 255u};
  uint8_t white[1 * 4] = {255u, 255u, 255u, 255u};
  uint8_t grey[1 * 4] = {199u, 199u, 199u, 255u};

  for (size_t src_index = 0; src_index < arraysize(src_targets); src_index++) {
    GLenum src_target = src_targets[src_index];
    for (size_t dest_index = 0; dest_index < arraysize(dest_targets);
         dest_index++) {
      GLenum dest_target = dest_targets[dest_index];

      CreateAndBindDestinationTextureAndFBO(dest_target);

      // Allocate source and destination textures.
      glBindTexture(src_target, textures_[0]);
      CreateBackingForTexture(src_target, src_width, src_height);

      glBindTexture(dest_target, textures_[1]);
      CreateBackingForTexture(dest_target, dest_width, dest_height);

      // The bottom left is red, bottom right is blue, top left is green, top
      // right is white.
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, src_target,
                             textures_[0], 0);
      glBindTexture(src_target, textures_[0]);
      for (GLint x = 0; x < src_width; ++x) {
        for (GLint y = 0; y < src_height; ++y) {
          uint8_t* data;
          if (x < src_width / 2) {
            data = y < src_height / 2 ? red : green;
          } else {
            data = y < src_height / 2 ? blue : white;
          }
          glTexSubImage2D(src_target, 0, x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                          data);
        }
      }

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, dest_target,
                             textures_[1], 0);
      glBindTexture(dest_target, textures_[1]);

      // Copy the subtexture x=[13,18) y=[6,9) to the destination.
      glClearColor(grey[0] / 255.f, grey[1] / 255.f, grey[2] / 255.f, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);
      glCopySubTextureCHROMIUM(textures_[0], 0, dest_target, textures_[1], 0,
                               copy_region_x, copy_region_y, 13, 6,
                               copy_region_width, copy_region_height, false,
                               false, false);
      EXPECT_TRUE(GL_NO_ERROR == glGetError());

      for (GLint x = 0; x < dest_width; ++x) {
        for (GLint y = 0; y < dest_height; ++y) {
          if (x < copy_region_x || x >= copy_region_x + copy_region_width ||
              y < copy_region_y || y >= copy_region_y + copy_region_height) {
            GLTestHelper::CheckPixels(x, y, 1, 1, 0, grey, nullptr);
            continue;
          }

          uint8_t* expected_color;
          if (x < copy_region_x + 2) {
            expected_color = y < copy_region_y + 1 ? red : green;
          } else {
            expected_color = y < copy_region_y + 1 ? blue : white;
          }
          GLTestHelper::CheckPixels(x, y, 1, 1, 0, expected_color, nullptr);
        }
      }

      glDeleteTextures(2, textures_);
      glDeleteFramebuffers(1, &framebuffer_id_);
    }
  }
}

}  // namespace gpu

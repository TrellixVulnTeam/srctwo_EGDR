// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/webgl/WebGLCompressedTextureS3TCsRGB.h"

#include "modules/webgl/WebGLRenderingContextBase.h"

namespace blink {

WebGLCompressedTextureS3TCsRGB::WebGLCompressedTextureS3TCsRGB(
    WebGLRenderingContextBase* context)
    : WebGLExtension(context) {
  // TODO(kainino): update these with _EXT versions once
  // GL_EXT_compressed_texture_s3tc_srgb is ratified
  context->AddCompressedTextureFormat(GL_COMPRESSED_SRGB_S3TC_DXT1_NV);
  context->AddCompressedTextureFormat(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_NV);
  context->AddCompressedTextureFormat(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_NV);
  context->AddCompressedTextureFormat(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_NV);
}

WebGLExtensionName WebGLCompressedTextureS3TCsRGB::GetName() const {
  return kWebGLCompressedTextureS3TCsRGBName;
}

WebGLCompressedTextureS3TCsRGB* WebGLCompressedTextureS3TCsRGB::Create(
    WebGLRenderingContextBase* context) {
  return new WebGLCompressedTextureS3TCsRGB(context);
}

bool WebGLCompressedTextureS3TCsRGB::Supported(
    WebGLRenderingContextBase* context) {
  Extensions3DUtil* extensions_util = context->ExtensionsUtil();
  return extensions_util->SupportsExtension(
      "GL_EXT_texture_compression_s3tc_srgb");
}

const char* WebGLCompressedTextureS3TCsRGB::ExtensionName() {
  return "WEBGL_compressed_texture_s3tc_srgb";
}

}  // namespace blink

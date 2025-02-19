// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/skia_bindings/grcontext_for_gles2_interface.h"

#include <stddef.h>
#include <string.h>
#include <utility>

#include "base/lazy_instance.h"
#include "base/macros.h"
#include "base/sys_info.h"
#include "base/trace_event/trace_event.h"
#include "gpu/command_buffer/client/gles2_interface.h"
#include "gpu/command_buffer/common/capabilities.h"
#include "gpu/skia_bindings/gl_bindings_skia_cmd_buffer.h"
#include "third_party/skia/include/gpu/GrContext.h"
#include "third_party/skia/include/gpu/GrContextOptions.h"
#include "third_party/skia/include/gpu/gl/GrGLInterface.h"

namespace skia_bindings {

GrContextForGLES2Interface::GrContextForGLES2Interface(
    gpu::gles2::GLES2Interface* gl,
    const gpu::Capabilities& capabilities) {
  // Calculate limits to pass during initialization:
  // The limit of the number of GPU resources we hold in the GrContext's
  // GPU cache.
  static const int kMaxGaneshResourceCacheCount = 16384;
  // The limit of the bytes allocated toward GPU resources in the GrContext's
  // GPU cache.
  static const size_t kMaxLowEndGaneshResourceCacheBytes = 48 * 1024 * 1024;
  static const size_t kMaxGaneshResourceCacheBytes = 96 * 1024 * 1024;
  static const size_t kMaxHighEndGaneshResourceCacheBytes = 256 * 1024 * 1024;
  // Limits for glyph cache textures.
  static const size_t kMaxDefaultGlyphCacheTextureBytes = 2048 * 1024 * 4;
  static const size_t kMaxLowEndGlyphCacheTextureBytes = 1024 * 512 * 4;
  // High-end / low-end memory cutoffs.
  static const int64_t kHighEndMemoryThreshold = (int64_t)4096 * 1024 * 1024;
  static const int64_t kLowEndMemoryThreshold = (int64_t)512 * 1024 * 1024;

  size_t max_ganesh_resource_cache_bytes = kMaxGaneshResourceCacheBytes;
  int max_glyph_cache_texture_bytes = kMaxDefaultGlyphCacheTextureBytes;
  if (base::SysInfo::AmountOfPhysicalMemory() <= kLowEndMemoryThreshold) {
    max_ganesh_resource_cache_bytes = kMaxLowEndGaneshResourceCacheBytes;
    max_glyph_cache_texture_bytes = kMaxLowEndGlyphCacheTextureBytes;
  } else if (base::SysInfo::AmountOfPhysicalMemory() >=
             kHighEndMemoryThreshold) {
    max_ganesh_resource_cache_bytes = kMaxHighEndGaneshResourceCacheBytes;
  }

  GrContextOptions options;
  options.fGlyphCacheTextureMaximumBytes = max_glyph_cache_texture_bytes;
  options.fAvoidStencilBuffers = capabilities.avoid_stencil_buffers;
  sk_sp<GrGLInterface> interface(
      skia_bindings::CreateGLES2InterfaceBindings(gl));
  gr_context_ = sk_sp<GrContext>(GrContext::Create(
      kOpenGL_GrBackend,
      // GrContext takes ownership of |interface|.
      reinterpret_cast<GrBackendContext>(interface.get()), options));
  if (gr_context_) {
    gr_context_->setResourceCacheLimits(kMaxGaneshResourceCacheCount,
                                        max_ganesh_resource_cache_bytes);
  }
}

GrContextForGLES2Interface::~GrContextForGLES2Interface() {
  // At this point the GLES2Interface is going to be destroyed, so have
  // the GrContext clean up and not try to use it anymore.
  if (gr_context_)
    gr_context_->releaseResourcesAndAbandonContext();
}

void GrContextForGLES2Interface::OnLostContext() {
  if (gr_context_)
    gr_context_->abandonContext();
}

void GrContextForGLES2Interface::ResetContext(uint32_t state) {
  if (gr_context_)
    gr_context_->resetContext(state);
}

void GrContextForGLES2Interface::FreeGpuResources() {
  if (gr_context_) {
    TRACE_EVENT_INSTANT0("gpu", "GrContext::freeGpuResources",
                         TRACE_EVENT_SCOPE_THREAD);
    gr_context_->freeGpuResources();
  }
}

}  // namespace skia_bindings

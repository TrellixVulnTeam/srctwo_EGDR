// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_IPC_SERVICE_GPU_MEMORY_BUFFER_FACTORY_NATIVE_PIXMAP_H_
#define GPU_IPC_SERVICE_GPU_MEMORY_BUFFER_FACTORY_NATIVE_PIXMAP_H_

#include <unordered_map>
#include <utility>

#include "base/hash.h"
#include "base/macros.h"
#include "base/synchronization/lock.h"
#include "gpu/command_buffer/service/image_factory.h"
#include "gpu/gpu_export.h"
#include "gpu/ipc/service/gpu_memory_buffer_factory.h"
#include "ui/gfx/native_pixmap.h"

namespace gl {
class GLImage;
}

namespace gpu {

class GPU_EXPORT GpuMemoryBufferFactoryNativePixmap
    : public GpuMemoryBufferFactory,
      public ImageFactory {
 public:
  GpuMemoryBufferFactoryNativePixmap();
  ~GpuMemoryBufferFactoryNativePixmap() override;

  // Overridden from GpuMemoryBufferFactory:
  gfx::GpuMemoryBufferHandle CreateGpuMemoryBuffer(
      gfx::GpuMemoryBufferId id,
      const gfx::Size& size,
      gfx::BufferFormat format,
      gfx::BufferUsage usage,
      int client_id,
      SurfaceHandle surface_handle) override;
  void DestroyGpuMemoryBuffer(gfx::GpuMemoryBufferId id,
                              int client_id) override;
  ImageFactory* AsImageFactory() override;

  // Overridden from ImageFactory:
  scoped_refptr<gl::GLImage> CreateImageForGpuMemoryBuffer(
      const gfx::GpuMemoryBufferHandle& handle,
      const gfx::Size& size,
      gfx::BufferFormat format,
      unsigned internalformat,
      int client_id,
      SurfaceHandle surface_handle) override;
  scoped_refptr<gl::GLImage> CreateAnonymousImage(
      const gfx::Size& size,
      gfx::BufferFormat format,
      unsigned internalformat) override;
  unsigned RequiredTextureType() override;

 private:
  using NativePixmapMapKey = std::pair<int, int>;
  using NativePixmapMapKeyHash = base::IntPairHash<NativePixmapMapKey>;
  using NativePixmapMap = std::unordered_map<NativePixmapMapKey,
                                             scoped_refptr<gfx::NativePixmap>,
                                             NativePixmapMapKeyHash>;
  NativePixmapMap native_pixmaps_;
  base::Lock native_pixmaps_lock_;

  DISALLOW_COPY_AND_ASSIGN(GpuMemoryBufferFactoryNativePixmap);
};

}  // namespace gpu

#endif  // GPU_IPC_SERVICE_GPU_MEMORY_BUFFER_FACTORY_NATIVE_PIXMAP_H_

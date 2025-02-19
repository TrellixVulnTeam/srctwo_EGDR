// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_IPC_CLIENT_GPU_MEMORY_BUFFER_IMPL_H_
#define GPU_IPC_CLIENT_GPU_MEMORY_BUFFER_IMPL_H_

#include <memory>

#include "base/callback.h"
#include "base/macros.h"
#include "gpu/command_buffer/common/sync_token.h"
#include "gpu/gpu_export.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/gpu_memory_buffer.h"

namespace gpu {

// Provides common implementation of a GPU memory buffer.
class GPU_EXPORT GpuMemoryBufferImpl : public gfx::GpuMemoryBuffer {
 public:
  typedef base::Callback<void(const gpu::SyncToken& sync)> DestructionCallback;

  ~GpuMemoryBufferImpl() override;

  // Creates an instance from the given |handle|. |size| and |internalformat|
  // should match what was used to allocate the |handle|. |callback|, if
  // non-null, is called when instance is deleted, which is not necessarily on
  // the same thread as this function was called on and instance was created on.
  static std::unique_ptr<GpuMemoryBufferImpl> CreateFromHandle(
      const gfx::GpuMemoryBufferHandle& handle,
      const gfx::Size& size,
      gfx::BufferFormat format,
      gfx::BufferUsage usage,
      const DestructionCallback& callback);

  // Overridden from gfx::GpuMemoryBuffer:
  gfx::Size GetSize() const override;
  gfx::BufferFormat GetFormat() const override;
  gfx::GpuMemoryBufferId GetId() const override;
  ClientBuffer AsClientBuffer() override;

  void set_destruction_sync_token(const gpu::SyncToken& sync_token) {
    destruction_sync_token_ = sync_token;
  }

 protected:
  GpuMemoryBufferImpl(gfx::GpuMemoryBufferId id,
                      const gfx::Size& size,
                      gfx::BufferFormat format,
                      const DestructionCallback& callback);

  const gfx::GpuMemoryBufferId id_;
  const gfx::Size size_;
  const gfx::BufferFormat format_;
  const DestructionCallback callback_;
  bool mapped_;
  gpu::SyncToken destruction_sync_token_;

 private:
  DISALLOW_COPY_AND_ASSIGN(GpuMemoryBufferImpl);
};

}  // namespace gpu

#endif  // GPU_IPC_CLIENT_GPU_MEMORY_BUFFER_IMPL_H_

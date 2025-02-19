// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/public/cpp/system/buffer.h"

namespace mojo {

// static
ScopedSharedBufferHandle SharedBufferHandle::Create(uint64_t num_bytes) {
  MojoCreateSharedBufferOptions options = {
      sizeof(options), MOJO_CREATE_SHARED_BUFFER_OPTIONS_FLAG_NONE};
  SharedBufferHandle handle;
  MojoCreateSharedBuffer(&options, num_bytes, handle.mutable_value());
  return MakeScopedHandle(handle);
}

ScopedSharedBufferHandle SharedBufferHandle::Clone(
    SharedBufferHandle::AccessMode access_mode) const {
  ScopedSharedBufferHandle result;
  if (!is_valid())
    return result;

  MojoDuplicateBufferHandleOptions options = {
      sizeof(options), MOJO_DUPLICATE_BUFFER_HANDLE_OPTIONS_FLAG_NONE};
  if (access_mode == AccessMode::READ_ONLY)
    options.flags |= MOJO_DUPLICATE_BUFFER_HANDLE_OPTIONS_FLAG_READ_ONLY;
  SharedBufferHandle result_handle;
  MojoDuplicateBufferHandle(value(), &options, result_handle.mutable_value());
  result.reset(result_handle);
  return result;
}

ScopedSharedBufferMapping SharedBufferHandle::Map(uint64_t size) const {
  return MapAtOffset(size, 0);
}

ScopedSharedBufferMapping SharedBufferHandle::MapAtOffset(
    uint64_t size,
    uint64_t offset) const {
  void* buffer = nullptr;
  MojoMapBuffer(value(), offset, size, &buffer, MOJO_MAP_BUFFER_FLAG_NONE);
  return ScopedSharedBufferMapping(buffer);
}

}  // namespace mojo

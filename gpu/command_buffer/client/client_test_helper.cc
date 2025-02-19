// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Tests for GLES2Implementation.

#include "gpu/command_buffer/client/client_test_helper.h"

#include <stddef.h>
#include <stdint.h>

#include <memory>

#include "gpu/command_buffer/client/cmd_buffer_helper.h"
#include "gpu/command_buffer/common/command_buffer.h"
#include "testing/gmock/include/gmock/gmock.h"

using ::testing::_;
using ::testing::Invoke;

namespace gpu {

FakeCommandBufferServiceBase::FakeCommandBufferServiceBase() {}

FakeCommandBufferServiceBase::~FakeCommandBufferServiceBase() {}

CommandBuffer::State FakeCommandBufferServiceBase::GetState() {
  return state_;
}

void FakeCommandBufferServiceBase::SetReleaseCount(uint64_t release_count) {
  state_.release_count = release_count;
}

// Get's the Id of the next transfer buffer that will be returned
// by CreateTransferBuffer. This is useful for testing expected ids.
int32_t FakeCommandBufferServiceBase::GetNextFreeTransferBufferId() {
  for (int32_t ii = 0; ii < kMaxTransferBuffers; ++ii) {
    if (!transfer_buffer_buffers_[ii].get()) {
      return kTransferBufferBaseId + ii;
    }
  }
  return -1;
}

void FakeCommandBufferServiceBase::SetGetBufferHelper(int transfer_buffer_id) {
  ++state_.set_get_buffer_count;
  state_.get_offset = 0;
  state_.token = 10000;  // All token checks in the tests should pass.
}

scoped_refptr<gpu::Buffer>
FakeCommandBufferServiceBase::CreateTransferBufferHelper(size_t size,
                                                         int32_t* id) {
  *id = GetNextFreeTransferBufferId();
  if (*id >= 0) {
    int32_t ndx = *id - kTransferBufferBaseId;
    std::unique_ptr<base::SharedMemory> shared_memory(new base::SharedMemory());
    shared_memory->CreateAndMapAnonymous(size);
    transfer_buffer_buffers_[ndx] =
        MakeBufferFromSharedMemory(std::move(shared_memory), size);
  }
  return GetTransferBuffer(*id);
}

void FakeCommandBufferServiceBase::DestroyTransferBufferHelper(int32_t id) {
  DCHECK_GE(id, kTransferBufferBaseId);
  DCHECK_LT(id, kTransferBufferBaseId + kMaxTransferBuffers);
  id -= kTransferBufferBaseId;
  transfer_buffer_buffers_[id] = NULL;
}

scoped_refptr<Buffer> FakeCommandBufferServiceBase::GetTransferBuffer(
    int32_t id) {
  if ((id < kTransferBufferBaseId) ||
      (id >= kTransferBufferBaseId + kMaxTransferBuffers))
    return nullptr;
  return transfer_buffer_buffers_[id - kTransferBufferBaseId];
}

void FakeCommandBufferServiceBase::FlushHelper(int32_t put_offset) {
  state_.get_offset = put_offset;
}

void FakeCommandBufferServiceBase::SetToken(int32_t token) {
  state_.token = token;
}

void FakeCommandBufferServiceBase::SetParseError(error::Error error) {
  state_.error = error;
}

void FakeCommandBufferServiceBase::SetContextLostReason(
    error::ContextLostReason reason) {
  state_.context_lost_reason = reason;
}

// GCC requires these declarations, but MSVC requires they not be present
#ifndef _MSC_VER
const int32_t FakeCommandBufferServiceBase::kTransferBufferBaseId;
const int32_t FakeCommandBufferServiceBase::kMaxTransferBuffers;
#endif

MockClientCommandBuffer::MockClientCommandBuffer() {
  DelegateToFake();
}

MockClientCommandBuffer::~MockClientCommandBuffer() {}

CommandBuffer::State MockClientCommandBuffer::GetLastState() {
  return GetState();
}

CommandBuffer::State MockClientCommandBuffer::WaitForTokenInRange(int32_t start,
                                                                  int32_t end) {
  return GetState();
}

CommandBuffer::State MockClientCommandBuffer::WaitForGetOffsetInRange(
    uint32_t set_get_buffer_count,
    int32_t start,
    int32_t end) {
  State state = GetState();
  EXPECT_EQ(set_get_buffer_count, state.set_get_buffer_count);
  if (state.get_offset != put_offset_) {
    FlushHelper(put_offset_);
    OnFlush();
    state = GetState();
  }
  return state;
}

void MockClientCommandBuffer::SetGetBuffer(int transfer_buffer_id) {
  SetGetBufferHelper(transfer_buffer_id);
}

scoped_refptr<gpu::Buffer> MockClientCommandBuffer::CreateTransferBuffer(
    size_t size,
    int32_t* id) {
  return CreateTransferBufferHelper(size, id);
}

void MockClientCommandBuffer::Flush(int32_t put_offset) {
  put_offset_ = put_offset;
}

void MockClientCommandBuffer::OrderingBarrier(int32_t put_offset) {
  put_offset_ = put_offset;
}

void MockClientCommandBuffer::DelegateToFake() {
  ON_CALL(*this, DestroyTransferBuffer(_))
      .WillByDefault(Invoke(
          this, &FakeCommandBufferServiceBase::DestroyTransferBufferHelper));
}

MockClientCommandBufferMockFlush::MockClientCommandBufferMockFlush() {
  DelegateToFake();
}

MockClientCommandBufferMockFlush::~MockClientCommandBufferMockFlush() {}

void MockClientCommandBufferMockFlush::DelegateToFake() {
  MockClientCommandBuffer::DelegateToFake();
  ON_CALL(*this, Flush(_))
      .WillByDefault(Invoke(this, &MockClientCommandBufferMockFlush::DoFlush));
}

void MockClientCommandBufferMockFlush::DoFlush(int32_t put_offset) {
  MockClientCommandBuffer::Flush(put_offset);
}

MockClientGpuControl::MockClientGpuControl() {}

MockClientGpuControl::~MockClientGpuControl() {}

}  // namespace gpu

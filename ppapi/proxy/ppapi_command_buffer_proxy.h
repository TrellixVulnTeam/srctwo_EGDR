// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PPAPI_PROXY_COMMAND_BUFFER_PROXY_H_
#define PPAPI_PROXY_COMMAND_BUFFER_PROXY_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>

#include "base/callback.h"
#include "base/containers/hash_tables.h"
#include "base/macros.h"
#include "gpu/command_buffer/client/gpu_control.h"
#include "gpu/command_buffer/common/command_buffer.h"
#include "gpu/command_buffer/common/command_buffer_id.h"
#include "gpu/command_buffer/common/command_buffer_shared.h"
#include "ppapi/proxy/plugin_dispatcher.h"
#include "ppapi/proxy/ppapi_proxy_export.h"
#include "ppapi/shared_impl/host_resource.h"

namespace IPC {
class Message;
}

namespace ppapi {
namespace proxy {

class SerializedHandle;

class PPAPI_PROXY_EXPORT PpapiCommandBufferProxy : public gpu::CommandBuffer,
                                                   public gpu::GpuControl {
 public:
  PpapiCommandBufferProxy(const HostResource& resource,
                          PluginDispatcher* dispatcher,
                          const gpu::Capabilities& capabilities,
                          const SerializedHandle& shared_state,
                          gpu::CommandBufferId command_buffer_id);
  ~PpapiCommandBufferProxy() override;

  // gpu::CommandBuffer implementation:
  State GetLastState() override;
  void Flush(int32_t put_offset) override;
  void OrderingBarrier(int32_t put_offset) override;
  State WaitForTokenInRange(int32_t start, int32_t end) override;
  State WaitForGetOffsetInRange(uint32_t set_get_buffer_count,
                                int32_t start,
                                int32_t end) override;
  void SetGetBuffer(int32_t transfer_buffer_id) override;
  scoped_refptr<gpu::Buffer> CreateTransferBuffer(size_t size,
                                                  int32_t* id) override;
  void DestroyTransferBuffer(int32_t id) override;

  // gpu::GpuControl implementation:
  void SetGpuControlClient(gpu::GpuControlClient*) override;
  gpu::Capabilities GetCapabilities() override;
  int32_t CreateImage(ClientBuffer buffer,
                      size_t width,
                      size_t height,
                      unsigned internalformat) override;
  void DestroyImage(int32_t id) override;
  void SignalQuery(uint32_t query, const base::Closure& callback) override;
  void SetLock(base::Lock*) override;
  void EnsureWorkVisible() override;
  gpu::CommandBufferNamespace GetNamespaceID() const override;
  gpu::CommandBufferId GetCommandBufferID() const override;
  void FlushPendingWork() override;
  uint64_t GenerateFenceSyncRelease() override;
  bool IsFenceSyncRelease(uint64_t release) override;
  bool IsFenceSyncFlushed(uint64_t release) override;
  bool IsFenceSyncFlushReceived(uint64_t release) override;
  bool IsFenceSyncReleased(uint64_t release) override;
  void SignalSyncToken(const gpu::SyncToken& sync_token,
                       const base::Closure& callback) override;
  void WaitSyncTokenHint(const gpu::SyncToken& sync_token) override;
  bool CanWaitUnverifiedSyncToken(const gpu::SyncToken& sync_token) override;
  void AddLatencyInfo(
      const std::vector<ui::LatencyInfo>& latency_info) override;

 private:
  bool Send(IPC::Message* msg);
  void UpdateState(const gpu::CommandBuffer::State& state, bool success);

  // Try to read an updated copy of the state from shared memory.
  void TryUpdateState();

  // The shared memory area used to update state.
  gpu::CommandBufferSharedState* shared_state() const;

  void FlushInternal();

  const gpu::CommandBufferId command_buffer_id_;

  gpu::Capabilities capabilities_;
  State last_state_;
  std::unique_ptr<base::SharedMemory> shared_state_shm_;

  HostResource resource_;
  PluginDispatcher* dispatcher_;

  base::Closure channel_error_callback_;

  InstanceData::FlushInfo *flush_info_;

  uint64_t next_fence_sync_release_;
  uint64_t pending_fence_sync_release_;
  uint64_t flushed_fence_sync_release_;
  uint64_t validated_fence_sync_release_;

  DISALLOW_COPY_AND_ASSIGN(PpapiCommandBufferProxy);
};

}  // namespace proxy
}  // namespace ppapi

#endif // PPAPI_PROXY_COMMAND_BUFFER_PROXY_H_

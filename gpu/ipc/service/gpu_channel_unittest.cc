// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include "gpu/ipc/common/gpu_messages.h"
#include "gpu/ipc/service/gpu_channel.h"
#include "gpu/ipc/service/gpu_channel_test_common.h"

namespace gpu {

class GpuChannelTest : public GpuChannelTestCommon {};

#if defined(OS_WIN)
const SurfaceHandle kFakeSurfaceHandle = reinterpret_cast<SurfaceHandle>(1);
#else
const SurfaceHandle kFakeSurfaceHandle = 1;
#endif

TEST_F(GpuChannelTest, CreateViewCommandBufferAllowed) {
  int32_t kClientId = 1;
  bool is_gpu_host = true;
  GpuChannel* channel = CreateChannel(kClientId, is_gpu_host);
  ASSERT_TRUE(channel);

  SurfaceHandle surface_handle = kFakeSurfaceHandle;
  DCHECK_NE(surface_handle, kNullSurfaceHandle);

  int32_t kRouteId = 1;
  GPUCreateCommandBufferConfig init_params;
  init_params.surface_handle = surface_handle;
  init_params.share_group_id = MSG_ROUTING_NONE;
  init_params.stream_id = 0;
  init_params.stream_priority = SchedulingPriority::kNormal;
  init_params.attribs = gles2::ContextCreationAttribHelper();
  init_params.active_url = GURL();
  bool result = false;
  gpu::Capabilities capabilities;
  HandleMessage(channel, new GpuChannelMsg_CreateCommandBuffer(
                             init_params, kRouteId, GetSharedHandle(), &result,
                             &capabilities));
  EXPECT_TRUE(result);

  GpuCommandBufferStub* stub = channel->LookupCommandBuffer(kRouteId);
  ASSERT_TRUE(stub);
}

TEST_F(GpuChannelTest, CreateViewCommandBufferDisallowed) {
  int32_t kClientId = 1;
  bool is_gpu_host = false;
  GpuChannel* channel = CreateChannel(kClientId, is_gpu_host);
  ASSERT_TRUE(channel);

  SurfaceHandle surface_handle = kFakeSurfaceHandle;
  DCHECK_NE(surface_handle, kNullSurfaceHandle);

  int32_t kRouteId = 1;
  GPUCreateCommandBufferConfig init_params;
  init_params.surface_handle = surface_handle;
  init_params.share_group_id = MSG_ROUTING_NONE;
  init_params.stream_id = 0;
  init_params.stream_priority = SchedulingPriority::kNormal;
  init_params.attribs = gles2::ContextCreationAttribHelper();
  init_params.active_url = GURL();
  bool result = false;
  gpu::Capabilities capabilities;
  HandleMessage(channel, new GpuChannelMsg_CreateCommandBuffer(
                             init_params, kRouteId, GetSharedHandle(), &result,
                             &capabilities));
  EXPECT_FALSE(result);

  GpuCommandBufferStub* stub = channel->LookupCommandBuffer(kRouteId);
  EXPECT_FALSE(stub);
}

TEST_F(GpuChannelTest, CreateOffscreenCommandBuffer) {
  int32_t kClientId = 1;
  GpuChannel* channel = CreateChannel(kClientId, true);
  ASSERT_TRUE(channel);

  int32_t kRouteId = 1;
  GPUCreateCommandBufferConfig init_params;
  init_params.surface_handle = kNullSurfaceHandle;
  init_params.share_group_id = MSG_ROUTING_NONE;
  init_params.stream_id = 0;
  init_params.stream_priority = SchedulingPriority::kNormal;
  init_params.attribs = gles2::ContextCreationAttribHelper();
  init_params.active_url = GURL();
  bool result = false;
  gpu::Capabilities capabilities;
  HandleMessage(channel, new GpuChannelMsg_CreateCommandBuffer(
                             init_params, kRouteId, GetSharedHandle(), &result,
                             &capabilities));
  EXPECT_TRUE(result);

  GpuCommandBufferStub* stub = channel->LookupCommandBuffer(kRouteId);
  EXPECT_TRUE(stub);
}

TEST_F(GpuChannelTest, IncompatibleStreamIds) {
  int32_t kClientId = 1;
  GpuChannel* channel = CreateChannel(kClientId, true);
  ASSERT_TRUE(channel);

  // Create first context.
  int32_t kRouteId1 = 1;
  int32_t kStreamId1 = 1;
  GPUCreateCommandBufferConfig init_params;
  init_params.surface_handle = kNullSurfaceHandle;
  init_params.share_group_id = MSG_ROUTING_NONE;
  init_params.stream_id = kStreamId1;
  init_params.stream_priority = SchedulingPriority::kNormal;
  init_params.attribs = gles2::ContextCreationAttribHelper();
  init_params.active_url = GURL();
  bool result = false;
  gpu::Capabilities capabilities;
  HandleMessage(channel, new GpuChannelMsg_CreateCommandBuffer(
                             init_params, kRouteId1, GetSharedHandle(), &result,
                             &capabilities));
  EXPECT_TRUE(result);

  GpuCommandBufferStub* stub = channel->LookupCommandBuffer(kRouteId1);
  EXPECT_TRUE(stub);

  // Create second context in same share group but different stream.
  int32_t kRouteId2 = 2;
  int32_t kStreamId2 = 2;

  init_params.share_group_id = kRouteId1;
  init_params.stream_id = kStreamId2;
  init_params.stream_priority = SchedulingPriority::kNormal;
  init_params.attribs = gles2::ContextCreationAttribHelper();
  init_params.active_url = GURL();
  HandleMessage(channel, new GpuChannelMsg_CreateCommandBuffer(
                             init_params, kRouteId2, GetSharedHandle(), &result,
                             &capabilities));
  EXPECT_FALSE(result);

  stub = channel->LookupCommandBuffer(kRouteId2);
  EXPECT_FALSE(stub);
}

TEST_F(GpuChannelTest, HighPriorityStreamsDisallowed) {
  int32_t kClientId = 1;
  bool is_gpu_host = false;
  GpuChannel* channel = CreateChannel(kClientId, is_gpu_host);
  ASSERT_TRUE(channel);

  // Highest priority is disallowed.
  int32_t kRouteId = 1;
  int32_t kStreamId = 1;
  GPUCreateCommandBufferConfig init_params;
  init_params.surface_handle = kNullSurfaceHandle;
  init_params.share_group_id = MSG_ROUTING_NONE;
  init_params.stream_id = kStreamId;
  init_params.stream_priority = SchedulingPriority::kHighest;
  init_params.attribs = gles2::ContextCreationAttribHelper();
  init_params.active_url = GURL();
  bool result = false;
  gpu::Capabilities capabilities;
  HandleMessage(channel, new GpuChannelMsg_CreateCommandBuffer(
                             init_params, kRouteId, GetSharedHandle(), &result,
                             &capabilities));
  EXPECT_FALSE(result);
  EXPECT_FALSE(channel->LookupCommandBuffer(kRouteId));

  // High priority is also disallowed.
  init_params.stream_priority = SchedulingPriority::kHigh;
  HandleMessage(channel, new GpuChannelMsg_CreateCommandBuffer(
                             init_params, kRouteId, GetSharedHandle(), &result,
                             &capabilities));
  EXPECT_FALSE(result);
  EXPECT_FALSE(channel->LookupCommandBuffer(kRouteId));
}

TEST_F(GpuChannelTest, HighPriorityStreamsAllowed) {
  int32_t kClientId = 1;
  bool is_gpu_host = true;
  GpuChannel* channel = CreateChannel(kClientId, is_gpu_host);
  ASSERT_TRUE(channel);

  // Highest priority is allowed.
  int32_t kRouteId1 = 1;
  GPUCreateCommandBufferConfig init_params;
  init_params.surface_handle = kNullSurfaceHandle;
  init_params.share_group_id = MSG_ROUTING_NONE;
  init_params.stream_id = 1;
  init_params.stream_priority = SchedulingPriority::kHighest;
  init_params.attribs = gles2::ContextCreationAttribHelper();
  init_params.active_url = GURL();
  bool result = false;
  gpu::Capabilities capabilities;
  HandleMessage(channel, new GpuChannelMsg_CreateCommandBuffer(
                             init_params, kRouteId1, GetSharedHandle(), &result,
                             &capabilities));
  EXPECT_TRUE(result);
  EXPECT_TRUE(channel->LookupCommandBuffer(kRouteId1));

  // High priority is also allowed.
  int32_t kRouteId2 = 2;
  init_params.stream_id = 2;
  init_params.stream_priority = SchedulingPriority::kHigh;
  result = false;
  HandleMessage(channel, new GpuChannelMsg_CreateCommandBuffer(
                             init_params, kRouteId2, GetSharedHandle(), &result,
                             &capabilities));
  EXPECT_TRUE(result);
  EXPECT_TRUE(channel->LookupCommandBuffer(kRouteId2));
}

TEST_F(GpuChannelTest, CreateFailsIfSharedContextIsLost) {
  int32_t kClientId = 1;
  GpuChannel* channel = CreateChannel(kClientId, false);
  ASSERT_TRUE(channel);

  // Create first context, we will share this one.
  int32_t kSharedRouteId = 1;
  {
    SCOPED_TRACE("kSharedRouteId");
    GPUCreateCommandBufferConfig init_params;
    init_params.surface_handle = kNullSurfaceHandle;
    init_params.share_group_id = MSG_ROUTING_NONE;
    init_params.stream_id = 0;
    init_params.stream_priority = SchedulingPriority::kNormal;
    init_params.attribs = gles2::ContextCreationAttribHelper();
    init_params.active_url = GURL();
    bool result = false;
    gpu::Capabilities capabilities;
    HandleMessage(channel, new GpuChannelMsg_CreateCommandBuffer(
                               init_params, kSharedRouteId, GetSharedHandle(),
                               &result, &capabilities));
    EXPECT_TRUE(result);
  }
  EXPECT_TRUE(channel->LookupCommandBuffer(kSharedRouteId));

  // This context shares with the first one, this should be possible.
  int32_t kFriendlyRouteId = 2;
  {
    SCOPED_TRACE("kFriendlyRouteId");
    GPUCreateCommandBufferConfig init_params;
    init_params.surface_handle = kNullSurfaceHandle;
    init_params.share_group_id = kSharedRouteId;
    init_params.stream_id = 0;
    init_params.stream_priority = SchedulingPriority::kNormal;
    init_params.attribs = gles2::ContextCreationAttribHelper();
    init_params.active_url = GURL();
    bool result = false;
    gpu::Capabilities capabilities;
    HandleMessage(channel, new GpuChannelMsg_CreateCommandBuffer(
                               init_params, kFriendlyRouteId, GetSharedHandle(),
                               &result, &capabilities));
    EXPECT_TRUE(result);
  }
  EXPECT_TRUE(channel->LookupCommandBuffer(kFriendlyRouteId));

  // The shared context is lost.
  channel->LookupCommandBuffer(kSharedRouteId)->MarkContextLost();

  // Meanwhile another context is being made pointing to the shared one. This
  // should fail.
  int32_t kAnotherRouteId = 3;
  {
    SCOPED_TRACE("kAnotherRouteId");
    GPUCreateCommandBufferConfig init_params;
    init_params.surface_handle = kNullSurfaceHandle;
    init_params.share_group_id = kSharedRouteId;
    init_params.stream_id = 0;
    init_params.stream_priority = SchedulingPriority::kNormal;
    init_params.attribs = gles2::ContextCreationAttribHelper();
    init_params.active_url = GURL();
    bool result = false;
    gpu::Capabilities capabilities;
    HandleMessage(channel, new GpuChannelMsg_CreateCommandBuffer(
                               init_params, kAnotherRouteId, GetSharedHandle(),
                               &result, &capabilities));
    EXPECT_FALSE(result);
  }
  EXPECT_FALSE(channel->LookupCommandBuffer(kAnotherRouteId));

  // The lost context is still around though (to verify the failure happened due
  // to the shared context being lost, not due to it being deleted).
  EXPECT_TRUE(channel->LookupCommandBuffer(kSharedRouteId));

  // Destroy the command buffers we initialized before destoying GL.
  HandleMessage(channel,
                new GpuChannelMsg_DestroyCommandBuffer(kFriendlyRouteId));
  HandleMessage(channel,
                new GpuChannelMsg_DestroyCommandBuffer(kSharedRouteId));
}

}  // namespace gpu

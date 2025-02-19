// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_TEST_FAKE_LAYER_TREE_FRAME_SINK_CLIENT_H_
#define CC_TEST_FAKE_LAYER_TREE_FRAME_SINK_CLIENT_H_

#include "cc/output/layer_tree_frame_sink_client.h"

#include "cc/output/managed_memory_policy.h"

namespace cc {

class FakeLayerTreeFrameSinkClient : public LayerTreeFrameSinkClient {
 public:
  FakeLayerTreeFrameSinkClient() : memory_policy_(0) {}

  void SetBeginFrameSource(viz::BeginFrameSource* source) override;
  void DidReceiveCompositorFrameAck() override;
  void ReclaimResources(
      const std::vector<viz::ReturnedResource>& resources) override {}
  void DidLoseLayerTreeFrameSink() override;
  void SetExternalTilePriorityConstraints(
      const gfx::Rect& viewport_rect_for_tile_priority,
      const gfx::Transform& transform_for_tile_priority) override {}
  void SetMemoryPolicy(const ManagedMemoryPolicy& policy) override;
  void SetTreeActivationCallback(const base::Closure&) override {}
  void OnDraw(const gfx::Transform& transform,
              const gfx::Rect& viewport,
              bool resourceless_software_draw) override {}

  int ack_count() { return ack_count_; }

  bool did_lose_layer_tree_frame_sink_called() {
    return did_lose_layer_tree_frame_sink_called_;
  }

  const ManagedMemoryPolicy& memory_policy() const { return memory_policy_; }

  viz::BeginFrameSource* begin_frame_source() const {
    return begin_frame_source_;
  }

 private:
  int ack_count_ = 0;
  bool did_lose_layer_tree_frame_sink_called_ = false;
  ManagedMemoryPolicy memory_policy_;
  viz::BeginFrameSource* begin_frame_source_;
};

}  // namespace cc

#endif  // CC_TEST_FAKE_LAYER_TREE_FRAME_SINK_CLIENT_H_

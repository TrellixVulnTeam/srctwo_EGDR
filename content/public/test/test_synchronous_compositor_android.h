// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_TEST_TEST_SYNCHRONOUS_COMPOSITOR_ANDROID_H_
#define CONTENT_PUBLIC_TEST_TEST_SYNCHRONOUS_COMPOSITOR_ANDROID_H_

#include <stddef.h>

#include <memory>
#include <vector>

#include "base/macros.h"
#include "content/public/browser/android/synchronous_compositor.h"
#include "content/public/browser/android/synchronous_compositor_client.h"

namespace content {

class CONTENT_EXPORT TestSynchronousCompositor : public SynchronousCompositor {
 public:
  TestSynchronousCompositor(int process_id, int routing_id);
  ~TestSynchronousCompositor() override;

  void SetClient(SynchronousCompositorClient* client);

  // SynchronousCompositor overrides.
  SynchronousCompositor::Frame DemandDrawHw(
      const gfx::Size& viewport_size,
      const gfx::Rect& viewport_rect_for_tile_priority,
      const gfx::Transform& transform_for_tile_priority) override;
  scoped_refptr<FrameFuture> DemandDrawHwAsync(
      const gfx::Size& viewport_size,
      const gfx::Rect& viewport_rect_for_tile_priority,
      const gfx::Transform& transform_for_tile_priority) override;
  void ReturnResources(
      uint32_t layer_tree_frame_sink_id,
      const std::vector<viz::ReturnedResource>& resources) override;
  bool DemandDrawSw(SkCanvas* canvas) override;
  void SetMemoryPolicy(size_t bytes_limit) override {}
  void DidChangeRootLayerScrollOffset(
      const gfx::ScrollOffset& root_offset) override {}
  void SynchronouslyZoomBy(float zoom_delta,
                           const gfx::Point& anchor) override {}
  void OnComputeScroll(base::TimeTicks animate_time) override {}

  void SetHardwareFrame(uint32_t layer_tree_frame_sink_id,
                        std::unique_ptr<cc::CompositorFrame> frame);

  struct ReturnedResources {
    ReturnedResources();
    ReturnedResources(const ReturnedResources& other);
    ~ReturnedResources();

    uint32_t layer_tree_frame_sink_id;
    std::vector<viz::ReturnedResource> resources;
  };
  using FrameAckArray = std::vector<ReturnedResources>;
  void SwapReturnedResources(FrameAckArray* array);

 private:
  SynchronousCompositorClient* client_;
  const int process_id_;
  const int routing_id_;
  SynchronousCompositor::Frame hardware_frame_;
  FrameAckArray frame_ack_array_;

  DISALLOW_COPY_AND_ASSIGN(TestSynchronousCompositor);
};

}  // namespace content

#endif  // CONTENT_PUBLIC_TEST_TEST_SYNCHRONOUS_COMPOSITOR_ANDROID_H_

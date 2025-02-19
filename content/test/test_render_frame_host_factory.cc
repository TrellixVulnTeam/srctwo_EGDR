// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/test/test_render_frame_host_factory.h"

#include "base/compiler_specific.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "content/test/test_render_frame_host.h"

namespace content {

TestRenderFrameHostFactory::TestRenderFrameHostFactory() {
  RenderFrameHostFactory::RegisterFactory(this);
}

TestRenderFrameHostFactory::~TestRenderFrameHostFactory() {
  RenderFrameHostFactory::UnregisterFactory();
}

std::unique_ptr<RenderFrameHostImpl>
TestRenderFrameHostFactory::CreateRenderFrameHost(
    SiteInstance* site_instance,
    RenderViewHostImpl* render_view_host,
    RenderFrameHostDelegate* delegate,
    RenderWidgetHostDelegate* rwh_delegate,
    FrameTree* frame_tree,
    FrameTreeNode* frame_tree_node,
    int32_t routing_id,
    int32_t widget_routing_id,
    bool hidden,
    bool renderer_initiated_creation) {
  return base::MakeUnique<TestRenderFrameHost>(
      site_instance, render_view_host, delegate, rwh_delegate, frame_tree,
      frame_tree_node, routing_id, widget_routing_id, hidden);
}

}  // namespace content

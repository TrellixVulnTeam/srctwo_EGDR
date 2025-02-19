// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/animation/CompositorAnimationHost.h"

#include "cc/animation/animation_host.h"
#include "cc/animation/scroll_offset_animations.h"
#include "platform/animation/CompositorAnimationTimeline.h"

namespace blink {

CompositorAnimationHost::CompositorAnimationHost(cc::AnimationHost* host)
    : animation_host_(host) {
  DCHECK(animation_host_);
}

void CompositorAnimationHost::AddTimeline(
    const CompositorAnimationTimeline& timeline) {
  animation_host_->AddAnimationTimeline(timeline.GetAnimationTimeline());
}

void CompositorAnimationHost::RemoveTimeline(
    const CompositorAnimationTimeline& timeline) {
  animation_host_->RemoveAnimationTimeline(timeline.GetAnimationTimeline());
}

void CompositorAnimationHost::AdjustImplOnlyScrollOffsetAnimation(
    CompositorElementId element_id,
    const gfx::Vector2dF& adjustment) {
  animation_host_->scroll_offset_animations().AddAdjustmentUpdate(element_id,
                                                                  adjustment);
}

void CompositorAnimationHost::TakeOverImplOnlyScrollOffsetAnimation(
    CompositorElementId element_id) {
  animation_host_->scroll_offset_animations().AddTakeoverUpdate(element_id);
}

}  // namespace blink

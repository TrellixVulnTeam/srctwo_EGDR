// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/animation/CompositorFilterAnimationCurve.h"

#include "cc/animation/keyframed_animation_curve.h"
#include "cc/animation/timing_function.h"
#include "cc/base/filter_operations.h"
#include "platform/graphics/CompositorFilterOperations.h"

namespace blink {

CompositorFilterAnimationCurve::CompositorFilterAnimationCurve()
    : curve_(cc::KeyframedFilterAnimationCurve::Create()) {}

CompositorFilterAnimationCurve::~CompositorFilterAnimationCurve() {}

void CompositorFilterAnimationCurve::AddKeyframe(
    const CompositorFilterKeyframe& keyframe) {
  curve_->AddKeyframe(keyframe.CloneToCC());
}

void CompositorFilterAnimationCurve::SetTimingFunction(
    const TimingFunction& timing_function) {
  curve_->SetTimingFunction(timing_function.CloneToCC());
}

void CompositorFilterAnimationCurve::SetScaledDuration(double scaled_duration) {
  curve_->set_scaled_duration(scaled_duration);
}

std::unique_ptr<cc::AnimationCurve>
CompositorFilterAnimationCurve::CloneToAnimationCurve() const {
  return curve_->Clone();
}

}  // namespace blink

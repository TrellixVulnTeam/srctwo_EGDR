// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/animation/animation.h"

#include "base/memory/ptr_util.h"
#include "build/build_config.h"
#include "ui/gfx/animation/animation_container.h"
#include "ui/gfx/animation/animation_delegate.h"
#include "ui/gfx/animation/tween.h"
#include "ui/gfx/geometry/rect.h"

namespace gfx {

// static
Animation::RichAnimationRenderMode Animation::rich_animation_rendering_mode_ =
    RichAnimationRenderMode::PLATFORM;

Animation::Animation(base::TimeDelta timer_interval)
    : timer_interval_(timer_interval),
      is_animating_(false),
      delegate_(NULL) {
}

Animation::~Animation() {
  // Don't send out notification from the destructor. Chances are the delegate
  // owns us and is being deleted as well.
  if (is_animating_)
    container_->Stop(this);
}

void Animation::Start() {
  if (is_animating_)
    return;

  if (!container_.get())
    container_ = new AnimationContainer();

  is_animating_ = true;

  container_->Start(this);

  AnimationStarted();
}

void Animation::Stop() {
  if (!is_animating_)
    return;

  is_animating_ = false;

  // Notify the container first as the delegate may delete us.
  container_->Stop(this);

  AnimationStopped();

  if (delegate_) {
    if (ShouldSendCanceledFromStop())
      delegate_->AnimationCanceled(this);
    else
      delegate_->AnimationEnded(this);
  }
}

double Animation::CurrentValueBetween(double start, double target) const {
  return Tween::DoubleValueBetween(GetCurrentValue(), start, target);
}

int Animation::CurrentValueBetween(int start, int target) const {
  return Tween::IntValueBetween(GetCurrentValue(), start, target);
}

gfx::Rect Animation::CurrentValueBetween(const gfx::Rect& start_bounds,
                                         const gfx::Rect& target_bounds) const {
  return Tween::RectValueBetween(
      GetCurrentValue(), start_bounds, target_bounds);
}

void Animation::SetContainer(AnimationContainer* container) {
  if (container == container_.get())
    return;

  if (is_animating_)
    container_->Stop(this);

  if (container)
    container_ = container;
  else
    container_ = new AnimationContainer();

  if (is_animating_)
    container_->Start(this);
}

bool Animation::ShouldRenderRichAnimation() {
  if (rich_animation_rendering_mode_ == RichAnimationRenderMode::PLATFORM)
    return ShouldRenderRichAnimationImpl();
  return rich_animation_rendering_mode_ ==
         RichAnimationRenderMode::FORCE_ENABLED;
}

#if !defined(OS_WIN)
// static
bool Animation::ShouldRenderRichAnimationImpl() {
  // Defined in platform specific file for Windows.
  return true;
}
#endif

#if !defined(OS_WIN) && !defined(OS_MACOSX)
// static
bool Animation::ScrollAnimationsEnabledBySystem() {
  // Defined in platform specific files for Windows and OSX.
  return true;
}
#endif

bool Animation::ShouldSendCanceledFromStop() {
  return false;
}

void Animation::SetStartTime(base::TimeTicks start_time) {
  start_time_ = start_time;
}

base::TimeDelta Animation::GetTimerInterval() const {
  return timer_interval_;
}

}  // namespace gfx

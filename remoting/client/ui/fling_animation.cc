// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/client/ui/fling_animation.h"

#include "base/time/default_tick_clock.h"

namespace remoting {

FlingAnimation::FlingAnimation(float time_constant,
                               const FlingCallback& fling_callback)
    : fling_tracker_(time_constant),
      fling_callback_(fling_callback),
      clock_(new base::DefaultTickClock()) {}

FlingAnimation::~FlingAnimation() {}

void FlingAnimation::SetVelocity(float velocity_x, float velocity_y) {
  fling_tracker_.StartFling(velocity_x, velocity_y);
  fling_start_time_ = clock_->NowTicks();
}

bool FlingAnimation::IsAnimationInProgress() const {
  return fling_tracker_.IsFlingInProgress();
}

void FlingAnimation::Tick() {
  if (!IsAnimationInProgress()) {
    return;
  }

  float dx, dy;
  bool in_progress = fling_tracker_.TrackMovement(
      clock_->NowTicks() - fling_start_time_, &dx, &dy);
  if (in_progress) {
    fling_callback_.Run(dx, dy);
  }
}

void FlingAnimation::Abort() {
  fling_tracker_.StopFling();
}

void FlingAnimation::SetTickClockForTest(
    std::unique_ptr<base::TickClock> clock) {
  clock_ = std::move(clock);
}

}  // namespace remoting

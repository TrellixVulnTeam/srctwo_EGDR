// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/renderer_host/input/synthetic_touchscreen_pinch_gesture.h"

#include <stdint.h>

#include <cmath>

#include "base/logging.h"
#include "ui/latency/latency_info.h"

namespace content {

SyntheticTouchscreenPinchGesture::SyntheticTouchscreenPinchGesture(
    const SyntheticPinchGestureParams& params)
    : params_(params),
      start_y_0_(0.0f),
      start_y_1_(0.0f),
      max_pointer_delta_0_(0.0f),
      gesture_source_type_(SyntheticGestureParams::DEFAULT_INPUT),
      state_(SETUP) {
  DCHECK_GT(params_.scale_factor, 0.0f);
}

SyntheticTouchscreenPinchGesture::~SyntheticTouchscreenPinchGesture() {}

SyntheticGesture::Result SyntheticTouchscreenPinchGesture::ForwardInputEvents(
    const base::TimeTicks& timestamp,
    SyntheticGestureTarget* target) {
  if (state_ == SETUP) {
    gesture_source_type_ = params_.gesture_source_type;
    if (gesture_source_type_ == SyntheticGestureParams::DEFAULT_INPUT)
      gesture_source_type_ = target->GetDefaultSyntheticGestureSourceType();

    state_ = STARTED;
    start_time_ = timestamp;
  }

  DCHECK_NE(gesture_source_type_, SyntheticGestureParams::DEFAULT_INPUT);

  if (!synthetic_pointer_driver_)
    synthetic_pointer_driver_ =
        SyntheticPointerDriver::Create(gesture_source_type_);

  if (gesture_source_type_ == SyntheticGestureParams::TOUCH_INPUT) {
    ForwardTouchInputEvents(timestamp, target);
  } else {
    return SyntheticGesture::GESTURE_SOURCE_TYPE_NOT_IMPLEMENTED;
  }

  return (state_ == DONE) ? SyntheticGesture::GESTURE_FINISHED
                          : SyntheticGesture::GESTURE_RUNNING;
}

void SyntheticTouchscreenPinchGesture::ForwardTouchInputEvents(
    const base::TimeTicks& timestamp,
    SyntheticGestureTarget* target) {
  switch (state_) {
    case STARTED:
      // Check for an early finish.
      if (params_.scale_factor == 1.0f) {
        state_ = DONE;
        break;
      }
      SetupCoordinatesAndStopTime(target);
      PressTouchPoints(target, timestamp);
      state_ = MOVING;
      break;
    case MOVING: {
      base::TimeTicks event_timestamp = ClampTimestamp(timestamp);
      float delta = GetDeltaForPointer0AtTime(event_timestamp);
      MoveTouchPoints(target, delta, event_timestamp);
      if (HasReachedTarget(event_timestamp)) {
        ReleaseTouchPoints(target, event_timestamp);
        state_ = DONE;
      }
    } break;
    case SETUP:
      NOTREACHED() << "State SETUP invalid for synthetic pinch.";
    case DONE:
      NOTREACHED() << "State DONE invalid for synthetic pinch.";
  }
}

void SyntheticTouchscreenPinchGesture::PressTouchPoints(
    SyntheticGestureTarget* target,
    const base::TimeTicks& timestamp) {
  synthetic_pointer_driver_->Press(params_.anchor.x(), start_y_0_, 0);
  synthetic_pointer_driver_->Press(params_.anchor.x(), start_y_1_, 1);
  synthetic_pointer_driver_->DispatchEvent(target, timestamp);
}

void SyntheticTouchscreenPinchGesture::MoveTouchPoints(
    SyntheticGestureTarget* target,
    float delta,
    const base::TimeTicks& timestamp) {
  // The two pointers move in opposite directions.
  float current_y_0 = start_y_0_ + delta;
  float current_y_1 = start_y_1_ - delta;

  synthetic_pointer_driver_->Move(params_.anchor.x(), current_y_0, 0);
  synthetic_pointer_driver_->Move(params_.anchor.x(), current_y_1, 1);
  synthetic_pointer_driver_->DispatchEvent(target, timestamp);
}

void SyntheticTouchscreenPinchGesture::ReleaseTouchPoints(
    SyntheticGestureTarget* target,
    const base::TimeTicks& timestamp) {
  synthetic_pointer_driver_->Release(0);
  synthetic_pointer_driver_->Release(1);
  synthetic_pointer_driver_->DispatchEvent(target, timestamp);
}

void SyntheticTouchscreenPinchGesture::SetupCoordinatesAndStopTime(
    SyntheticGestureTarget* target) {
  // To achieve the specified scaling factor, the ratio of the final to the
  // initial span (distance between the pointers) has to be equal to the scaling
  // factor. Since we're moving both pointers at the same speed, each pointer's
  // distance to the anchor is half the span.
  float initial_distance_to_anchor, final_distance_to_anchor;
  if (params_.scale_factor > 1.0f) {  // zooming in
    initial_distance_to_anchor = target->GetMinScalingSpanInDips() / 2.0f;
    final_distance_to_anchor =
        (initial_distance_to_anchor + target->GetTouchSlopInDips()) *
        params_.scale_factor;
  } else {  // zooming out
    final_distance_to_anchor = target->GetMinScalingSpanInDips() / 2.0f;
    initial_distance_to_anchor =
        (final_distance_to_anchor / params_.scale_factor) +
        target->GetTouchSlopInDips();
  }

  start_y_0_ = params_.anchor.y() - initial_distance_to_anchor;
  start_y_1_ = params_.anchor.y() + initial_distance_to_anchor;

  max_pointer_delta_0_ = initial_distance_to_anchor - final_distance_to_anchor;

  int64_t total_duration_in_us = static_cast<int64_t>(
      1e6 * (static_cast<double>(std::abs(2 * max_pointer_delta_0_)) /
             params_.relative_pointer_speed_in_pixels_s));
  DCHECK_GT(total_duration_in_us, 0);
  stop_time_ =
      start_time_ + base::TimeDelta::FromMicroseconds(total_duration_in_us);
}

float SyntheticTouchscreenPinchGesture::GetDeltaForPointer0AtTime(
    const base::TimeTicks& timestamp) const {
  // Make sure the final delta is correct. Using the computation below can lead
  // to issues with floating point precision.
  if (HasReachedTarget(timestamp))
    return max_pointer_delta_0_;

  float total_abs_delta = params_.relative_pointer_speed_in_pixels_s *
                          (timestamp - start_time_).InSecondsF();
  float abs_delta_pointer_0 = total_abs_delta / 2.0f;
  return (params_.scale_factor > 1.0f) ? -abs_delta_pointer_0
                                       : abs_delta_pointer_0;
}

base::TimeTicks SyntheticTouchscreenPinchGesture::ClampTimestamp(
    const base::TimeTicks& timestamp) const {
  return std::min(timestamp, stop_time_);
}

bool SyntheticTouchscreenPinchGesture::HasReachedTarget(
    const base::TimeTicks& timestamp) const {
  return timestamp >= stop_time_;
}

}  // namespace content

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_RENDERER_HOST_INPUT_SYNTHETIC_TOUCHSCREEN_PINCH_GESTURE_H_
#define CONTENT_BROWSER_RENDERER_HOST_INPUT_SYNTHETIC_TOUCHSCREEN_PINCH_GESTURE_H_

#include "base/macros.h"
#include "base/time/time.h"
#include "content/browser/renderer_host/input/synthetic_gesture.h"
#include "content/browser/renderer_host/input/synthetic_gesture_target.h"
#include "content/browser/renderer_host/input/synthetic_pointer_driver.h"
#include "content/common/content_export.h"
#include "content/common/input/synthetic_pinch_gesture_params.h"
#include "content/common/input/synthetic_web_input_event_builders.h"
#include "third_party/WebKit/public/platform/WebInputEvent.h"

namespace content {

class CONTENT_EXPORT SyntheticTouchscreenPinchGesture
    : public SyntheticGesture {
 public:
  explicit SyntheticTouchscreenPinchGesture(
      const SyntheticPinchGestureParams& params);
  ~SyntheticTouchscreenPinchGesture() override;

  SyntheticGesture::Result ForwardInputEvents(
      const base::TimeTicks& timestamp,
      SyntheticGestureTarget* target) override;

 private:
  enum GestureState { SETUP, STARTED, MOVING, DONE };

  void ForwardTouchInputEvents(const base::TimeTicks& timestamp,
                               SyntheticGestureTarget* target);

  void PressTouchPoints(SyntheticGestureTarget* target,
                        const base::TimeTicks& timestamp);
  void MoveTouchPoints(SyntheticGestureTarget* target,
                       float delta,
                       const base::TimeTicks& timestamp);
  void ReleaseTouchPoints(SyntheticGestureTarget* target,
                          const base::TimeTicks& timestamp);

  void SetupCoordinatesAndStopTime(SyntheticGestureTarget* target);
  float GetDeltaForPointer0AtTime(const base::TimeTicks& timestamp) const;
  base::TimeTicks ClampTimestamp(const base::TimeTicks& timestamp) const;
  bool HasReachedTarget(const base::TimeTicks& timestamp) const;

  SyntheticPinchGestureParams params_;
  std::unique_ptr<SyntheticPointerDriver> synthetic_pointer_driver_;
  float start_y_0_;
  float start_y_1_;
  float max_pointer_delta_0_;
  SyntheticGestureParams::GestureSourceType gesture_source_type_;
  GestureState state_;
  base::TimeTicks start_time_;
  base::TimeTicks stop_time_;

 private:
  DISALLOW_COPY_AND_ASSIGN(SyntheticTouchscreenPinchGesture);
};

}  // namespace content

#endif  // CONTENT_BROWSER_RENDERER_HOST_INPUT_SYNTHETIC_TOUCHSCREEN_PINCH_GESTURE_H_

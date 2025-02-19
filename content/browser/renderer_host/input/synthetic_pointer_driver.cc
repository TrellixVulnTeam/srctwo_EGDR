// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/renderer_host/input/synthetic_pointer_driver.h"

#include "base/memory/ptr_util.h"
#include "content/browser/renderer_host/input/synthetic_mouse_driver.h"
#include "content/browser/renderer_host/input/synthetic_pen_driver.h"
#include "content/browser/renderer_host/input/synthetic_touch_driver.h"

namespace content {

SyntheticPointerDriver::SyntheticPointerDriver() {}
SyntheticPointerDriver::~SyntheticPointerDriver() {}

// static
std::unique_ptr<SyntheticPointerDriver> SyntheticPointerDriver::Create(
    SyntheticGestureParams::GestureSourceType gesture_source_type) {
  switch (gesture_source_type) {
    case SyntheticGestureParams::TOUCH_INPUT:
      return base::MakeUnique<SyntheticTouchDriver>();
    case SyntheticGestureParams::MOUSE_INPUT:
      return base::MakeUnique<SyntheticMouseDriver>();
    case SyntheticGestureParams::PEN_INPUT:
      return base::MakeUnique<SyntheticPenDriver>();
    case SyntheticGestureParams::DEFAULT_INPUT:
      return std::unique_ptr<SyntheticPointerDriver>();
  }
  NOTREACHED();
  return std::unique_ptr<SyntheticPointerDriver>();
}

// static
double SyntheticPointerDriver::ConvertTimestampToSeconds(
    const base::TimeTicks& timestamp) {
  return (timestamp - base::TimeTicks()).InSecondsF();
}

}  // namespace content

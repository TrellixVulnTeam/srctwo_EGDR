// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/input/InputDeviceCapabilities.h"

namespace blink {

InputDeviceCapabilities::InputDeviceCapabilities(bool fires_touch_events) {
  fires_touch_events_ = fires_touch_events;
}

InputDeviceCapabilities::InputDeviceCapabilities(
    const InputDeviceCapabilitiesInit& initializer) {
  fires_touch_events_ = initializer.firesTouchEvents();
}

InputDeviceCapabilities* InputDeviceCapabilitiesConstants::FiresTouchEvents(
    bool fires_touch) {
  if (fires_touch) {
    if (!fires_touch_events_)
      fires_touch_events_ = InputDeviceCapabilities::Create(true);
    return fires_touch_events_;
  }
  if (!doesnt_fire_touch_events_)
    doesnt_fire_touch_events_ = InputDeviceCapabilities::Create(false);
  return doesnt_fire_touch_events_;
}

}  // namespace blink

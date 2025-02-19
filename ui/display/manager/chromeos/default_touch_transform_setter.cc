// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/display/manager/chromeos/default_touch_transform_setter.h"

#include "ui/events/devices/device_data_manager.h"
#include "ui/events/devices/touch_device_transform.h"

namespace display {

DefaultTouchTransformSetter::DefaultTouchTransformSetter() = default;

DefaultTouchTransformSetter::~DefaultTouchTransformSetter() = default;

void DefaultTouchTransformSetter::ConfigureTouchDevices(
    const std::vector<ui::TouchDeviceTransform>& transforms) {
  ui::DeviceDataManager::GetInstance()->ConfigureTouchDevices(transforms);
}

}  // namespace display

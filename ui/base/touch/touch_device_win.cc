// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/touch/touch_device.h"

#include "base/logging.h"
#include "base/win/win_util.h"

namespace ui {

namespace {

bool IsTouchDevicePresent() {
  int value = GetSystemMetrics(SM_DIGITIZER);
  return (value & NID_READY) &&
      ((value & NID_INTEGRATED_TOUCH) || (value & NID_EXTERNAL_TOUCH));
}

}  // namespace

// The following method logic is as follow :
// - On versions prior to Windows 8 it will always return POINTER_TYPE_FINE
// and/or POINTER_TYPE_COARSE (if the device has a touch screen).
// - If the device is a detachable/convertible win8/10 device and the keyboard/
// trackpad is detached/flipped it will always return POINTER_TYPE_COARSE.
// It does not cover the case where an external mouse/keyboard is connected
// while the device is used as a tablet. This is because Windows doesn't provide
// us a reliable way to detect keyboard/mouse presence with
// GetSystemMetrics(SM_MOUSEPRESENT).
// - If the device doesn't have a touch screen it will return POINTER_TYPE_FINE.
// In the rare cases (this is Microsoft documentation) where
// GetSystemMetrics(SM_MOUSEPRESENT) returns 0 we will return POINTER_TYPE_NONE.
// - If the device has a touch screen the available pointer devices are
// POINTER_TYPE_FINE and POINTER_TYPE_COARSE.
int GetAvailablePointerTypes() {
  // IsTabletDevice guarantees us that :
  // - The device has a touch screen.
  // - It is used as a tablet which means that it has no keyboard connected.
  // On Windows 10 it means that it is verifying with ConvertibleSlateMode.
  if (base::win::IsTabletDevice(nullptr))
    return POINTER_TYPE_COARSE;

  if (GetSystemMetrics(SM_MOUSEPRESENT) == 0)
    return POINTER_TYPE_NONE;

  int available_pointer_types = POINTER_TYPE_FINE;
  if (IsTouchDevicePresent())
    available_pointer_types |= POINTER_TYPE_COARSE;

  return available_pointer_types;
}

// This method follows the same logic as above but with hover types.
int GetAvailableHoverTypes() {
  if (base::win::IsTabletDevice(nullptr))
    return HOVER_TYPE_NONE;

  int available_hover_types;
  if (GetSystemMetrics(SM_MOUSEPRESENT) != 0) {
    available_hover_types = HOVER_TYPE_HOVER;
    if (IsTouchDevicePresent())
      available_hover_types |= HOVER_TYPE_NONE;
  } else
    available_hover_types = HOVER_TYPE_NONE;

  return available_hover_types;
}

TouchScreensAvailability GetTouchScreensAvailability() {
  if (!IsTouchDevicePresent())
    return TouchScreensAvailability::NONE;

  return TouchScreensAvailability::ENABLED;
}

int MaxTouchPoints() {
  if (!IsTouchDevicePresent())
    return 0;

  return GetSystemMetrics(SM_MAXIMUMTOUCHES);
}

PointerType GetPrimaryPointerType(int available_pointer_types) {
  if (available_pointer_types & POINTER_TYPE_FINE)
    return POINTER_TYPE_FINE;
  if (available_pointer_types & POINTER_TYPE_COARSE)
    return POINTER_TYPE_COARSE;
  DCHECK_EQ(available_pointer_types, POINTER_TYPE_NONE);
  return POINTER_TYPE_NONE;
}

HoverType GetPrimaryHoverType(int available_hover_types) {
  if (available_hover_types & HOVER_TYPE_HOVER)
    return HOVER_TYPE_HOVER;
  DCHECK_EQ(available_hover_types, HOVER_TYPE_NONE);
  return HOVER_TYPE_NONE;
}

}  // namespace ui

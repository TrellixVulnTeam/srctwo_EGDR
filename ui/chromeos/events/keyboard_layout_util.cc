// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/chromeos/events/keyboard_layout_util.h"

#include "ui/chromeos/events/event_rewriter_chromeos.h"
#include "ui/events/devices/input_device_manager.h"

namespace ui {

bool DeviceUsesKeyboardLayout2() {
  for (const InputDevice& keyboard :
       InputDeviceManager::GetInstance()->GetKeyboardDevices()) {
    if (keyboard.type == InputDeviceType::INPUT_DEVICE_INTERNAL &&
        EventRewriterChromeOS::GetKeyboardTopRowLayout(keyboard.sys_path) ==
            EventRewriterChromeOS::kKbdTopRowLayout2) {
      return true;
    }
  }

  return false;
}

}  // namespace ui
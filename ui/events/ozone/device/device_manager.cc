// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/events/ozone/device/device_manager.h"

#include "base/memory/ptr_util.h"
#include "base/trace_event/trace_event.h"

#if defined(USE_UDEV)
#include "ui/events/ozone/device/udev/device_manager_udev.h"
#else
#include "ui/events/ozone/device/device_manager_manual.h"
#endif

namespace ui {

std::unique_ptr<DeviceManager> CreateDeviceManager() {
  TRACE_EVENT0("ozone", "CreateDeviceManager");
#if defined(USE_UDEV)
  return base::MakeUnique<DeviceManagerUdev>();
#else
  return base::MakeUnique<DeviceManagerManual>();
#endif
}

}  // namespace ui

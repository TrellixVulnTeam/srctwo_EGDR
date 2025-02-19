// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/display_info_provider_win.h"

#include <windows.h>
#include <stddef.h>

#include "base/hash.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/win_util.h"
#include "extensions/common/api/system_display.h"
#include "ui/display/display.h"
#include "ui/display/win/dpi.h"
#include "ui/gfx/geometry/size.h"

namespace extensions {

using api::system_display::DisplayUnitInfo;

namespace {

BOOL CALLBACK
EnumMonitorCallback(HMONITOR monitor, HDC hdc, LPRECT rect, LPARAM data) {
  DisplayInfoProvider::DisplayUnitInfoList* all_displays =
      reinterpret_cast<DisplayInfoProvider::DisplayUnitInfoList*>(data);
  DCHECK(all_displays);

  DisplayUnitInfo unit;

  MONITORINFOEX monitor_info;
  ZeroMemory(&monitor_info, sizeof(MONITORINFOEX));
  monitor_info.cbSize = sizeof(monitor_info);
  GetMonitorInfo(monitor, &monitor_info);

  DISPLAY_DEVICE device;
  device.cb = sizeof(device);
  if (!EnumDisplayDevices(monitor_info.szDevice, 0, &device, 0))
    return FALSE;

  unit.id =
      base::Int64ToString(base::Hash(base::WideToUTF8(monitor_info.szDevice)));
  unit.name = base::WideToUTF8(device.DeviceString);
  all_displays->push_back(std::move(unit));

  return TRUE;
}

}  // namespace

DisplayInfoProviderWin::DisplayInfoProviderWin() {
}

DisplayInfoProviderWin::~DisplayInfoProviderWin() {
}

bool DisplayInfoProviderWin::SetInfo(
    const std::string& display_id,
    const api::system_display::DisplayProperties& info,
    std::string* error) {
  *error = "Not implemented";
  return false;
}

void DisplayInfoProviderWin::UpdateDisplayUnitInfoForPlatform(
    const display::Display& display,
    extensions::api::system_display::DisplayUnitInfo* unit) {
  DisplayUnitInfoList all_displays;
  EnumDisplayMonitors(
      NULL, NULL, EnumMonitorCallback, reinterpret_cast<LPARAM>(&all_displays));
  for (size_t i = 0; i < all_displays.size(); ++i) {
    if (unit->id == all_displays[i].id) {
      unit->name = all_displays[i].name;
      float device_scale_factor = display.device_scale_factor();
      int dpi = display::win::GetDPIFromScalingFactor(device_scale_factor);
      unit->dpi_x = dpi;
      unit->dpi_y = dpi;
      break;
    }
  }
}

// static
DisplayInfoProvider* DisplayInfoProvider::Create() {
  return new DisplayInfoProviderWin();
}

}  // namespace extensions

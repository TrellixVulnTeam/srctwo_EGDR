// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/display/win/color_profile_reader.h"

#include <stddef.h>
#include <windows.h>

#include "base/files/file_util.h"
#include "base/task_scheduler/post_task.h"
#include "base/threading/sequenced_worker_pool.h"
#include "ui/display/win/display_info.h"
#include "ui/gfx/icc_profile.h"

namespace display {
namespace win {
namespace {

BOOL CALLBACK EnumMonitorCallback(HMONITOR monitor,
                                  HDC input_hdc,
                                  LPRECT rect,
                                  LPARAM data) {
  base::string16 device_name;
  MONITORINFOEX monitor_info;
  ::ZeroMemory(&monitor_info, sizeof(monitor_info));
  monitor_info.cbSize = sizeof(monitor_info);
  ::GetMonitorInfo(monitor, &monitor_info);
  device_name = base::string16(monitor_info.szDevice);

  base::string16 profile_path;
  HDC hdc = ::CreateDC(monitor_info.szDevice, NULL, NULL, NULL);
  if (hdc) {
    DWORD path_length = MAX_PATH;
    WCHAR path[MAX_PATH + 1];
    BOOL result = ::GetICMProfile(hdc, &path_length, path);
    ::DeleteDC(hdc);
    if (result)
      profile_path = base::string16(path);
  }

  std::map<base::string16, base::string16>* device_to_path_map =
      reinterpret_cast<std::map<base::string16, base::string16>*>(data);
  (*device_to_path_map)[device_name] = profile_path;
  return TRUE;
}

}  // namespace

ColorProfileReader::ColorProfileReader(Client* client)
    : client_(client), weak_factory_(this) {}

ColorProfileReader::~ColorProfileReader() {}

void ColorProfileReader::UpdateIfNeeded() {
  if (update_in_flight_)
    return;

  DeviceToPathMap new_device_to_path_map = BuildDeviceToPathMap();
  if (device_to_path_map_ == new_device_to_path_map)
    return;

  if (!base::SequencedWorkerPool::IsEnabled())
    return;

  update_in_flight_ = true;
  base::PostTaskWithTraitsAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::BACKGROUND},
      base::Bind(&ColorProfileReader::ReadProfilesOnBackgroundThread,
                 new_device_to_path_map),
      base::Bind(&ColorProfileReader::ReadProfilesCompleted,
                 weak_factory_.GetWeakPtr()));
}

// static
ColorProfileReader::DeviceToPathMap ColorProfileReader::BuildDeviceToPathMap() {
  DeviceToPathMap device_to_path_map;
  EnumDisplayMonitors(nullptr, nullptr, EnumMonitorCallback,
                      reinterpret_cast<LPARAM>(&device_to_path_map));
  return device_to_path_map;
}

// static
ColorProfileReader::DeviceToDataMap
ColorProfileReader::ReadProfilesOnBackgroundThread(
    DeviceToPathMap new_device_to_path_map) {
  DeviceToDataMap new_device_to_data_map;
  for (auto entry : new_device_to_path_map) {
    const base::string16& device_name = entry.first;
    const base::string16& profile_path = entry.second;
    std::string profile_data;
    base::ReadFileToString(base::FilePath(profile_path), &profile_data);
    new_device_to_data_map[device_name] = profile_data;
  }
  return new_device_to_data_map;
}

void ColorProfileReader::ReadProfilesCompleted(
    DeviceToDataMap device_to_data_map) {
  DCHECK(update_in_flight_);
  update_in_flight_ = false;

  display_id_to_color_space_map_.clear();
  for (auto entry : device_to_data_map) {
    const base::string16& device_name = entry.first;
    const std::string& profile_data = entry.second;
    int64_t display_id =
        DisplayInfo::DeviceIdFromDeviceName(device_name.c_str());

    if (profile_data.empty()) {
      display_id_to_color_space_map_[display_id] = default_color_space_;
    } else {
      gfx::ICCProfile icc_profile =
          gfx::ICCProfile::FromData(profile_data.data(), profile_data.size());
      icc_profile.HistogramDisplay(display_id);
      display_id_to_color_space_map_[display_id] = icc_profile.GetColorSpace();
    }
  }

  client_->OnColorProfilesChanged();
}

const gfx::ColorSpace& ColorProfileReader::GetDisplayColorSpace(
    int64_t display_id) const {
  auto found = display_id_to_color_space_map_.find(display_id);
  if (found == display_id_to_color_space_map_.end())
    return default_color_space_;
  return found->second;
}

}  // namespace win
}  // namespace display

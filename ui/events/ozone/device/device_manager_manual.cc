// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/events/ozone/device/device_manager_manual.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/files/file_enumerator.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/task_scheduler/post_task.h"
#include "ui/events/ozone/device/device_event.h"
#include "ui/events/ozone/device/device_event_observer.h"

namespace ui {

namespace {

const char kDevInput[] = "/dev/input";

void ScanDevicesOnWorkerThread(std::vector<base::FilePath>* result) {
  base::FileEnumerator file_enum(base::FilePath(FILE_PATH_LITERAL(kDevInput)),
                                 false, base::FileEnumerator::FILES,
                                 FILE_PATH_LITERAL("event*[0-9]"));
  for (base::FilePath path = file_enum.Next(); !path.empty();
       path = file_enum.Next()) {
    result->push_back(path);
  }
}
}

DeviceManagerManual::DeviceManagerManual() : weak_ptr_factory_(this) {}

DeviceManagerManual::~DeviceManagerManual() {
}

void DeviceManagerManual::ScanDevices(DeviceEventObserver* observer) {
  if (!is_watching_) {
    is_watching_ = true;
    StartWatching();
  }

  InitiateScanDevices();
}

void DeviceManagerManual::AddObserver(DeviceEventObserver* observer) {
  observers_.AddObserver(observer);
}

void DeviceManagerManual::RemoveObserver(DeviceEventObserver* observer) {
  observers_.RemoveObserver(observer);
}

void DeviceManagerManual::StartWatching() {
  if (!watcher_.Watch(base::FilePath(FILE_PATH_LITERAL(kDevInput)), false,
                      base::Bind(&DeviceManagerManual::OnWatcherEvent,
                                 weak_ptr_factory_.GetWeakPtr()))) {
    LOG(ERROR) << "Failed to start FilePathWatcher";
  }
}

void DeviceManagerManual::InitiateScanDevices() {
  std::vector<base::FilePath>* result = new std::vector<base::FilePath>();
  base::PostTaskWithTraitsAndReply(
      FROM_HERE,
      {base::MayBlock(), base::TaskShutdownBehavior::CONTINUE_ON_SHUTDOWN},
      base::Bind(&ScanDevicesOnWorkerThread, result),
      base::Bind(&DeviceManagerManual::OnDevicesScanned,
                 weak_ptr_factory_.GetWeakPtr(), base::Owned(result)));
}

void DeviceManagerManual::OnDevicesScanned(
    std::vector<base::FilePath>* result) {
  std::set<base::FilePath> new_devices;

  // Reported newly added devices.
  for (const auto& path : *result) {
    new_devices.insert(path);
    // Don't report devices already added.
    if (devices_.find(path) != devices_.end())
      continue;

    DeviceEvent event(DeviceEvent::INPUT, DeviceEvent::ADD, path);
    for (DeviceEventObserver& observer : observers_) {
      observer.OnDeviceEvent(event);
    }
  }

  // Report removed devices.
  for (const auto& path : devices_) {
    if (new_devices.find(path) != new_devices.end())
      continue;

    DeviceEvent event(DeviceEvent::INPUT, DeviceEvent::REMOVE, path);
    for (DeviceEventObserver& observer : observers_) {
      observer.OnDeviceEvent(event);
    }
  }

  devices_.swap(new_devices);
}

void DeviceManagerManual::OnWatcherEvent(const base::FilePath& path,
                                         bool error) {
  // We need to restart watching if there's an error.
  if (error) {
    StartWatching();
  }
  InitiateScanDevices();
}

}  // namespace ui

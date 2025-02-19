// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// StorageMonitorCros listens for mount point changes and notifies listeners
// about the addition and deletion of media devices. This class lives on the
// UI thread.

#ifndef COMPONENTS_STORAGE_MONITOR_STORAGE_MONITOR_CHROMEOS_H_
#define COMPONENTS_STORAGE_MONITOR_STORAGE_MONITOR_CHROMEOS_H_

#if !defined(OS_CHROMEOS)
#error "Should only be used on ChromeOS."
#endif

#include <map>
#include <memory>
#include <string>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "build/build_config.h"
#include "chromeos/disks/disk_mount_manager.h"
#include "components/storage_monitor/storage_monitor.h"

namespace storage_monitor {

class MediaTransferProtocolDeviceObserverChromeOS;

class StorageMonitorCros : public StorageMonitor,
                           public chromeos::disks::DiskMountManager::Observer {
 public:
  // Should only be called by browser start up code.
  // Use StorageMonitor::GetInstance() instead.
  StorageMonitorCros();
  ~StorageMonitorCros() override;

  // Sets up disk listeners and issues notifications for any discovered
  // mount points. Sets up MTP manager and listeners.
  void Init() override;

 protected:
  void SetMediaTransferProtocolManagerForTest(
      device::MediaTransferProtocolManager* test_manager);

  // chromeos::disks::DiskMountManager::Observer implementation.
  void OnDiskEvent(
      chromeos::disks::DiskMountManager::DiskEvent event,
      const chromeos::disks::DiskMountManager::Disk* disk) override;
  void OnDeviceEvent(chromeos::disks::DiskMountManager::DeviceEvent event,
                     const std::string& device_path) override;
  void OnMountEvent(chromeos::disks::DiskMountManager::MountEvent event,
                    chromeos::MountError error_code,
                    const chromeos::disks::DiskMountManager::MountPointInfo&
                        mount_info) override;
  void OnFormatEvent(chromeos::disks::DiskMountManager::FormatEvent event,
                     chromeos::FormatError error_code,
                     const std::string& device_path) override;
  void OnRenameEvent(chromeos::disks::DiskMountManager::RenameEvent event,
                     chromeos::RenameError error_code,
                     const std::string& device_path) override;

  // StorageMonitor implementation.
  bool GetStorageInfoForPath(const base::FilePath& path,
                             StorageInfo* device_info) const override;
  void EjectDevice(const std::string& device_id,
                   base::Callback<void(EjectStatus)> callback) override;
  device::MediaTransferProtocolManager* media_transfer_protocol_manager()
      override;

 private:
  // Mapping of mount path to removable mass storage info.
  typedef std::map<std::string, StorageInfo> MountMap;

  // Helper method that checks existing mount points to see if they are media
  // devices. Eventually calls AddMountedPath for all mount points.
  void CheckExistingMountPoints();

  // Adds the mount point in |mount_info| to |mount_map_| and send a media
  // device attach notification. |has_dcim| is true if the attached device has
  // a DCIM folder.
  void AddMountedPath(
      const chromeos::disks::DiskMountManager::MountPointInfo& mount_info,
      bool has_dcim);

  // Mapping of relevant mount points and their corresponding mount devices.
  MountMap mount_map_;

  std::unique_ptr<device::MediaTransferProtocolManager>
      media_transfer_protocol_manager_;
  std::unique_ptr<MediaTransferProtocolDeviceObserverChromeOS>
      media_transfer_protocol_device_observer_;

  base::WeakPtrFactory<StorageMonitorCros> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(StorageMonitorCros);
};

}  // namespace storage_monitor

#endif  // COMPONENTS_STORAGE_MONITOR_STORAGE_MONITOR_CHROMEOS_H_

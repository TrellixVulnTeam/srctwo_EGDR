// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_COMPONENTS_TETHER_KEEP_ALIVE_SCHEDULER_H_
#define CHROMEOS_COMPONENTS_TETHER_KEEP_ALIVE_SCHEDULER_H_

#include <memory>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/timer/timer.h"
#include "chromeos/components/tether/active_host.h"
#include "chromeos/components/tether/device_status_util.h"
#include "chromeos/components/tether/keep_alive_operation.h"

namespace chromeos {

namespace tether {

class HostScanCache;
class DeviceIdTetherNetworkGuidMap;

// Schedules keep-alive messages to be sent when this device is connected to a
// remote device's tether hotspot. When a device connects, a keep-alive message
// is sent immediately, then another one is scheduled every 4 minutes until the
// device disconnects.
class KeepAliveScheduler : public ActiveHost::Observer,
                           public KeepAliveOperation::Observer {
 public:
  KeepAliveScheduler(
      ActiveHost* active_host,
      BleConnectionManager* connection_manager,
      HostScanCache* host_scan_cache,
      DeviceIdTetherNetworkGuidMap* device_id_tether_network_guid_map);
  virtual ~KeepAliveScheduler();

  // ActiveHost::Observer:
  void OnActiveHostChanged(
      const ActiveHost::ActiveHostChangeInfo& change_info) override;

  // KeepAliveOperation::Observer:
  void OnOperationFinished(
      const cryptauth::RemoteDevice& remote_device,
      std::unique_ptr<DeviceStatus> device_status) override;

 private:
  friend class KeepAliveSchedulerTest;

  KeepAliveScheduler(
      ActiveHost* active_host,
      BleConnectionManager* connection_manager,
      HostScanCache* host_scan_cache,
      DeviceIdTetherNetworkGuidMap* device_id_tether_network_guid_map,
      std::unique_ptr<base::Timer> timer);

  void SendKeepAliveTickle();

  static const uint32_t kKeepAliveIntervalMinutes;

  ActiveHost* active_host_;
  BleConnectionManager* connection_manager_;
  HostScanCache* host_scan_cache_;
  DeviceIdTetherNetworkGuidMap* device_id_tether_network_guid_map_;

  std::unique_ptr<base::Timer> timer_;
  std::shared_ptr<cryptauth::RemoteDevice> active_host_device_;
  std::unique_ptr<KeepAliveOperation> keep_alive_operation_;

  base::WeakPtrFactory<KeepAliveScheduler> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(KeepAliveScheduler);
};

}  // namespace tether

}  // namespace chromeos

#endif  // CHROMEOS_COMPONENTS_TETHER_KEEP_ALIVE_SCHEDULER_H_

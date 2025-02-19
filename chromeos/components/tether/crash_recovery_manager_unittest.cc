// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/components/tether/crash_recovery_manager.h"

#include <memory>
#include <sstream>

#include "base/bind.h"
#include "base/memory/ptr_util.h"
#include "base/test/scoped_task_environment.h"
#include "chromeos/components/tether/device_id_tether_network_guid_map.h"
#include "chromeos/components/tether/fake_active_host.h"
#include "chromeos/components/tether/fake_host_scan_cache.h"
#include "chromeos/components/tether/host_scan_cache_entry.h"
#include "chromeos/dbus/dbus_thread_manager.h"
#include "chromeos/network/network_state_handler.h"
#include "chromeos/network/network_state_test.h"
#include "components/cryptauth/remote_device_test_util.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/cros_system_api/dbus/shill/dbus-constants.h"

namespace chromeos {

namespace tether {

namespace {

constexpr char kWifiNetworkGuid[] = "wifiNetworkGuid";

std::string CreateConfigurationJsonString(bool is_connected) {
  std::string connection_state =
      is_connected ? shill::kStateOnline : shill::kStateIdle;

  std::stringstream ss;
  ss << "{"
     << "  \"GUID\": \"" << kWifiNetworkGuid << "\","
     << "  \"Type\": \"" << shill::kTypeWifi << "\","
     << "  \"State\": \"" << connection_state << "\""
     << "}";
  return ss.str();
}

}  // namespace

class CrashRecoveryManagerTest : public NetworkStateTest {
 protected:
  CrashRecoveryManagerTest()
      : test_device_(cryptauth::GenerateTestRemoteDevices(1u)[0]) {}
  ~CrashRecoveryManagerTest() override {}

  void SetUp() override {
    DBusThreadManager::Initialize();
    NetworkStateTest::SetUp();
    network_state_handler()->SetTetherTechnologyState(
        NetworkStateHandler::TECHNOLOGY_ENABLED);

    is_restoration_finished_ = false;

    fake_active_host_ = base::MakeUnique<FakeActiveHost>();
    fake_host_scan_cache_ = base::MakeUnique<FakeHostScanCache>();

    crash_recovery_manager_ = base::MakeUnique<CrashRecoveryManager>(
        network_state_handler(), fake_active_host_.get(),
        fake_host_scan_cache_.get());

    device_id_tether_network_guid_map_ =
        base::MakeUnique<DeviceIdTetherNetworkGuidMap>();
  }

  void TearDown() override {
    ShutdownNetworkState();
    NetworkStateTest::TearDown();
    DBusThreadManager::Shutdown();
  }

  std::string GetTetherNetworkGuid() {
    return device_id_tether_network_guid_map_->GetTetherNetworkGuidForDeviceId(
        test_device_.GetDeviceId());
  }

  void SetConnected() {
    fake_active_host_->SetActiveHostConnected(
        test_device_.GetDeviceId(), GetTetherNetworkGuid(), kWifiNetworkGuid);
  }

  void AddScanEntry() {
    fake_host_scan_cache_->SetHostScanResult(
        *HostScanCacheEntry::Builder()
             .SetTetherNetworkGuid(GetTetherNetworkGuid())
             .SetDeviceName("deviceName")
             .SetCarrier("carrier")
             .SetBatteryPercentage(100)
             .SetSignalStrength(100)
             .SetSetupRequired(false)
             .Build());

    network_state_handler()->AddTetherNetworkState(
        GetTetherNetworkGuid(), "deviceName", "carrier",
        100 /* battery_percentage */, 100 /* signal_strength */,
        false /* has_connected_to_host */);
  }

  void AddWifiNetwork(bool is_connected) {
    ConfigureService(CreateConfigurationJsonString(is_connected));
  }

  void StartRestoration() {
    crash_recovery_manager_->RestorePreCrashStateIfNecessary(
        base::Bind(&CrashRecoveryManagerTest::OnRestorationFinished,
                   base::Unretained(this)));
  }

  void OnRestorationFinished() { is_restoration_finished_ = true; }

  void VerifyDisconnectedAfterRestoration() {
    EXPECT_TRUE(is_restoration_finished_);
    EXPECT_EQ(ActiveHost::ActiveHostStatus::DISCONNECTED,
              fake_active_host_->GetActiveHostStatus());
  }

  const base::test::ScopedTaskEnvironment scoped_task_environment_;
  const cryptauth::RemoteDevice test_device_;

  std::unique_ptr<FakeActiveHost> fake_active_host_;
  std::unique_ptr<FakeHostScanCache> fake_host_scan_cache_;

  // TODO(hansberry): Use a fake for this when a real mapping scheme is created.
  std::unique_ptr<DeviceIdTetherNetworkGuidMap>
      device_id_tether_network_guid_map_;

  bool is_restoration_finished_;

  std::unique_ptr<CrashRecoveryManager> crash_recovery_manager_;

 private:
  DISALLOW_COPY_AND_ASSIGN(CrashRecoveryManagerTest);
};

TEST_F(CrashRecoveryManagerTest, ActiveHostDisconnected) {
  StartRestoration();
  VerifyDisconnectedAfterRestoration();
}

TEST_F(CrashRecoveryManagerTest, ActiveHostConnecting) {
  fake_active_host_->SetActiveHostConnecting(test_device_.GetDeviceId(),
                                             GetTetherNetworkGuid());

  StartRestoration();
  VerifyDisconnectedAfterRestoration();
}

TEST_F(CrashRecoveryManagerTest, ActiveHostConnected_EntryNotInCache) {
  SetConnected();

  StartRestoration();
  VerifyDisconnectedAfterRestoration();
}

TEST_F(CrashRecoveryManagerTest, ActiveHostConnected_WifiNetworkMissing) {
  AddScanEntry();
  SetConnected();

  StartRestoration();
  VerifyDisconnectedAfterRestoration();
}

TEST_F(CrashRecoveryManagerTest, ActiveHostConnected_WifiNetworkDisconnected) {
  AddScanEntry();
  AddWifiNetwork(false /* is_connected */);
  SetConnected();

  StartRestoration();
  VerifyDisconnectedAfterRestoration();
}

TEST_F(CrashRecoveryManagerTest, ActiveHostConnected_RestoreSuccessful) {
  AddScanEntry();
  AddWifiNetwork(true /* is_connected */);
  SetConnected();

  StartRestoration();
  EXPECT_TRUE(is_restoration_finished_);
  EXPECT_EQ(ActiveHost::ActiveHostStatus::CONNECTED,
            fake_active_host_->GetActiveHostStatus());
}

}  // namespace tether

}  // namespace chromeos

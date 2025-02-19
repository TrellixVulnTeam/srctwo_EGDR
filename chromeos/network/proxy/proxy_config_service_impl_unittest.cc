// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/network/proxy/proxy_config_service_impl.h"

#include <memory>

#include "base/memory/ptr_util.h"
#include "base/test/scoped_task_environment.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/values.h"
#include "chromeos/dbus/dbus_thread_manager.h"
#include "chromeos/network/network_handler.h"
#include "chromeos/network/network_state_test.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/testing_pref_service.h"
#include "components/proxy_config/pref_proxy_config_tracker_impl.h"
#include "components/proxy_config/proxy_config_pref_names.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace chromeos {
namespace {

const char kFixedPacUrl[] = "http://fixed/";

class TestProxyConfigService : public net::ProxyConfigService {
 public:
  TestProxyConfigService(const net::ProxyConfig& config,
                         ConfigAvailability availability)
      : config_(config), availability_(availability) {}

 private:
  void AddObserver(net::ProxyConfigService::Observer* observer) override {}
  void RemoveObserver(net::ProxyConfigService::Observer* observer) override {}

  net::ProxyConfigService::ConfigAvailability GetLatestProxyConfig(
      net::ProxyConfig* config) override {
    *config = config_;
    return availability_;
  }

  net::ProxyConfig config_;
  ConfigAvailability availability_;
};

}  // namespace

class ProxyConfigServiceImplTest : public NetworkStateTest {
  void SetUp() override {
    DBusThreadManager::Initialize();
    chromeos::NetworkHandler::Initialize();
    NetworkStateTest::SetUp();
  }

  void TearDown() override {
    NetworkStateTest::TearDown();
    chromeos::NetworkHandler::Shutdown();
    DBusThreadManager::Shutdown();
  }

 protected:
  base::test::ScopedTaskEnvironment environment_;
};

// By default, ProxyConfigServiceImpl should ignore the state of the nested
// ProxyConfigService.
TEST_F(ProxyConfigServiceImplTest, IgnoresNestedProxyConfigServiceByDefault) {
  TestingPrefServiceSimple profile_prefs;
  PrefProxyConfigTrackerImpl::RegisterProfilePrefs(profile_prefs.registry());
  TestingPrefServiceSimple local_state_prefs;

  net::ProxyConfig fixed_config;
  fixed_config.set_pac_url(GURL(kFixedPacUrl));
  std::unique_ptr<TestProxyConfigService> nested_service =
      base::MakeUnique<TestProxyConfigService>(
          fixed_config, net::ProxyConfigService::CONFIG_VALID);

  ProxyConfigServiceImpl proxy_tracker(&profile_prefs, &local_state_prefs,
                                       base::ThreadTaskRunnerHandle::Get());

  std::unique_ptr<net::ProxyConfigService> proxy_service =
      proxy_tracker.CreateTrackingProxyConfigService(std::move(nested_service));

  net::ProxyConfig config;
  EXPECT_EQ(net::ProxyConfigService::CONFIG_VALID,
            proxy_service->GetLatestProxyConfig(&config));
  EXPECT_TRUE(config.Equals(net::ProxyConfig::CreateDirect()));

  environment_.RunUntilIdle();
  EXPECT_EQ(net::ProxyConfigService::CONFIG_VALID,
            proxy_service->GetLatestProxyConfig(&config));
  EXPECT_TRUE(config.Equals(net::ProxyConfig::CreateDirect()));

  proxy_tracker.DetachFromPrefService();
}

// Sets proxy_config::prefs::kUseSharedProxies to true, and makes sure the
// nested ProxyConfigService is used.
TEST_F(ProxyConfigServiceImplTest, UsesNestedProxyConfigService) {
  TestingPrefServiceSimple profile_prefs;
  PrefProxyConfigTrackerImpl::RegisterProfilePrefs(profile_prefs.registry());
  TestingPrefServiceSimple local_state_prefs;
  profile_prefs.SetUserPref(proxy_config::prefs::kUseSharedProxies,
                            base::MakeUnique<base::Value>(true));

  net::ProxyConfig fixed_config;
  fixed_config.set_pac_url(GURL(kFixedPacUrl));
  std::unique_ptr<TestProxyConfigService> nested_service =
      base::MakeUnique<TestProxyConfigService>(
          fixed_config, net::ProxyConfigService::CONFIG_VALID);

  ProxyConfigServiceImpl proxy_tracker(&profile_prefs, &local_state_prefs,
                                       base::ThreadTaskRunnerHandle::Get());

  std::unique_ptr<net::ProxyConfigService> proxy_service =
      proxy_tracker.CreateTrackingProxyConfigService(std::move(nested_service));

  net::ProxyConfig config;
  EXPECT_EQ(net::ProxyConfigService::CONFIG_VALID,
            proxy_service->GetLatestProxyConfig(&config));
  EXPECT_TRUE(config.Equals(fixed_config));

  environment_.RunUntilIdle();
  EXPECT_EQ(net::ProxyConfigService::CONFIG_VALID,
            proxy_service->GetLatestProxyConfig(&config));
  EXPECT_TRUE(config.Equals(fixed_config));

  proxy_tracker.DetachFromPrefService();
}

}  // namespace chromeos

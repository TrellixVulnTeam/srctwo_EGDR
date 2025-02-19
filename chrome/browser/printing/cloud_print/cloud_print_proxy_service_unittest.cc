// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/printing/cloud_print/cloud_print_proxy_service.h"

#include <stdint.h>

#include <memory>
#include <string>

#include "base/command_line.h"
#include "base/location.h"
#include "base/memory/ptr_util.h"
#include "base/run_loop.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "chrome/browser/printing/cloud_print/cloud_print_proxy_service_factory.h"
#include "chrome/browser/service_process/service_process_control.h"
#include "chrome/browser/ui/startup/startup_browser_creator.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/cloud_print.mojom.h"
#include "chrome/common/cloud_print/cloud_print_proxy_info.h"
#include "chrome/common/pref_names.h"
#include "chrome/test/base/testing_browser_process.h"
#include "chrome/test/base/testing_profile.h"
#include "chrome/test/base/testing_profile_manager.h"
#include "components/prefs/testing_pref_service.h"
#include "components/sync_preferences/testing_pref_service_syncable.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "mojo/public/cpp/bindings/binding_set.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::Assign;
using ::testing::AtMost;
using ::testing::DeleteArg;
using ::testing::DoAll;
using ::testing::Invoke;
using ::testing::Property;
using ::testing::Return;
using ::testing::ReturnPointee;
using ::testing::WithArgs;
using ::testing::WithoutArgs;
using ::testing::_;

class MockServiceProcessControl : public ServiceProcessControl {
 public:
  static std::string EnabledUserId();

  MockServiceProcessControl() : connected_(false) { }

  MOCK_CONST_METHOD0(IsConnected, bool());

  MOCK_METHOD2(Launch, void(const base::Closure&, const base::Closure&));
  MOCK_METHOD0(Disconnect, void());

  void SetConnectSuccessMockExpectations(bool post_task);

 private:
  bool connected_;
  cloud_print::CloudPrintProxyInfo info_;
};

// static
std::string MockServiceProcessControl::EnabledUserId() {
  return std::string("dorothy@somewhere.otr");
}

void CallTask(const base::Closure& task) {
  if (!task.is_null())
    task.Run();
}

void PostTask(const base::Closure& task) {
  if (!task.is_null())
    base::ThreadTaskRunnerHandle::Get()->PostTask(FROM_HERE, task);
}

void MockServiceProcessControl::SetConnectSuccessMockExpectations(
    bool post_task) {
  EXPECT_CALL(*this, IsConnected()).WillRepeatedly(ReturnPointee(&connected_));

  EXPECT_CALL(*this, Launch(_, _))
      .WillRepeatedly(
          DoAll(Assign(&connected_, true),
                WithArgs<0>(Invoke(post_task ? PostTask : CallTask))));

  EXPECT_CALL(*this, Disconnect()).Times(AtMost(1))
      .WillRepeatedly(Assign(&connected_, false));

}

class MockCloudPrintProxy : public cloud_print::mojom::CloudPrint {
 public:
  void AddBinding(cloud_print::mojom::CloudPrintRequest request) {
    bindings_.AddBinding(this, std::move(request));
  }

  void ReturnDisabledInfo() {
    cloud_proxy_info_expectation_set_ = true;
    cloud_proxy_info_.enabled = false;
    cloud_proxy_info_.email.clear();
  }

  void ReturnEnabledInfo() {
    cloud_proxy_info_expectation_set_ = true;
    cloud_proxy_info_.enabled = true;
    cloud_proxy_info_.email = MockServiceProcessControl::EnabledUserId();
  }

  bool has_been_enabled() {
    bindings_.FlushForTesting();
    return enabled_;
  }
  bool has_been_disabled() {
    bindings_.FlushForTesting();
    return disabled_;
  }

 private:
  void GetCloudPrintProxyInfo(
      GetCloudPrintProxyInfoCallback callback) override {
    EXPECT_TRUE(cloud_proxy_info_expectation_set_);
    std::move(callback).Run(cloud_proxy_info_.enabled, cloud_proxy_info_.email,
                            cloud_proxy_info_.proxy_id);
  }
  void GetPrinters(GetPrintersCallback callback) override { NOTREACHED(); }
  void DisableCloudPrintProxy() override { disabled_ = true; }

  void EnableCloudPrintProxyWithRobot(
      const std::string& robot_auth_code,
      const std::string& robot_email,
      const std::string& user_email,
      std::unique_ptr<base::DictionaryValue> user_settings) override {
    enabled_ = true;
  }

  mojo::BindingSet<cloud_print::mojom::CloudPrint> bindings_;

  bool cloud_proxy_info_expectation_set_ = false;
  cloud_print::CloudPrintProxyInfo cloud_proxy_info_;

  bool disabled_ = false;
  bool enabled_ = false;
};

class TestCloudPrintProxyService : public CloudPrintProxyService {
 public:
  explicit TestCloudPrintProxyService(Profile* profile)
      : CloudPrintProxyService(profile) {
    service_manager::InterfaceProvider::TestApi test_api(
        &process_control_.remote_interfaces());
    test_api.SetBinderForName(
        "cloud_print::mojom::CloudPrint",
        base::Bind(&TestCloudPrintProxyService::HandleCloudPrintProxyRequest,
                   base::Unretained(this)));
    service_manager::mojom::InterfaceProviderPtr handle;
    mojo::MakeRequest(&handle);
    process_control_.SetMojoHandle(std::move(handle));
  }

  ~TestCloudPrintProxyService() override {
    service_manager::InterfaceProvider::TestApi test_api(
        &ServiceProcessControl::GetInstance()->remote_interfaces());
    test_api.ClearBinderForName("cloud_print::mojom::CloudPrint");
  }

  void Initialize() {
    CloudPrintProxyService::Initialize();
    base::RunLoop().RunUntilIdle();
  }

  void RefreshStatusFromService() {
    CloudPrintProxyService::RefreshStatusFromService();
    base::RunLoop().RunUntilIdle();
  }

  ServiceProcessControl* GetServiceProcessControl() override {
    return &process_control_;
  }

  MockServiceProcessControl* GetMockServiceProcessControl() {
    return &process_control_;
  }

  MockCloudPrintProxy& GetMockCloudPrintProxy() { return mock_proxy_; }

  void EnableForUser() {
    EnableForUserWithRobot("123", "123@gmail.com",
                           MockServiceProcessControl::EnabledUserId(),
                           base::DictionaryValue());
  }

 private:
  void HandleCloudPrintProxyRequest(mojo::ScopedMessagePipeHandle handle) {
    mock_proxy_.AddBinding(
        cloud_print::mojom::CloudPrintRequest(std::move(handle)));
  }

  MockServiceProcessControl process_control_;
  MockCloudPrintProxy mock_proxy_;
};

class CloudPrintProxyPolicyTest : public ::testing::Test {
 public:
  CloudPrintProxyPolicyTest() = default;

  bool LaunchBrowser(const base::CommandLine& command_line, Profile* profile) {
    StartupBrowserCreator browser_creator;
    return browser_creator.ProcessCmdLineImpl(
        command_line, base::FilePath(), false, profile,
        StartupBrowserCreator::Profiles());
  }

 protected:
  content::TestBrowserThreadBundle test_browser_thread_bundle_;
  TestingProfile profile_;
};

TEST_F(CloudPrintProxyPolicyTest, VerifyExpectations) {
  MockServiceProcessControl mock_control;
  mock_control.SetConnectSuccessMockExpectations(false);

  EXPECT_FALSE(mock_control.IsConnected());
  mock_control.Launch(base::Closure(), base::Closure());
  EXPECT_TRUE(mock_control.IsConnected());
  mock_control.Launch(base::Closure(), base::Closure());
  EXPECT_TRUE(mock_control.IsConnected());
  mock_control.Disconnect();
  EXPECT_FALSE(mock_control.IsConnected());
}

TEST_F(CloudPrintProxyPolicyTest, StartWithNoPolicyProxyDisabled) {
  TestCloudPrintProxyService service(&profile_);

  service.GetMockCloudPrintProxy().ReturnDisabledInfo();
  service.GetMockServiceProcessControl()->SetConnectSuccessMockExpectations(
      false);

  sync_preferences::TestingPrefServiceSyncable* prefs =
      profile_.GetTestingPrefService();
  prefs->SetUserPref(prefs::kCloudPrintEmail,
                     base::MakeUnique<base::Value>(
                         MockServiceProcessControl::EnabledUserId()));

  service.Initialize();

  EXPECT_EQ(std::string(), prefs->GetString(prefs::kCloudPrintEmail));
}

TEST_F(CloudPrintProxyPolicyTest, StartWithNoPolicyProxyEnabled) {
  TestCloudPrintProxyService service(&profile_);

  service.GetMockServiceProcessControl()->SetConnectSuccessMockExpectations(
      false);
  service.GetMockCloudPrintProxy().ReturnEnabledInfo();

  sync_preferences::TestingPrefServiceSyncable* prefs =
      profile_.GetTestingPrefService();
  prefs->SetUserPref(prefs::kCloudPrintEmail,
                     base::MakeUnique<base::Value>(std::string()));

  service.Initialize();
  service.RefreshStatusFromService();

  EXPECT_EQ(MockServiceProcessControl::EnabledUserId(),
            prefs->GetString(prefs::kCloudPrintEmail));
}

TEST_F(CloudPrintProxyPolicyTest, StartWithPolicySetProxyDisabled) {
  TestCloudPrintProxyService service(&profile_);

  service.GetMockServiceProcessControl()->SetConnectSuccessMockExpectations(
      false);
  service.GetMockCloudPrintProxy().ReturnDisabledInfo();

  sync_preferences::TestingPrefServiceSyncable* prefs =
      profile_.GetTestingPrefService();
  prefs->SetUserPref(prefs::kCloudPrintEmail,
                     base::MakeUnique<base::Value>(std::string()));
  prefs->SetManagedPref(prefs::kCloudPrintProxyEnabled,
                        base::MakeUnique<base::Value>(false));

  service.Initialize();

  EXPECT_EQ(std::string(), prefs->GetString(prefs::kCloudPrintEmail));
}

TEST_F(CloudPrintProxyPolicyTest, StartWithPolicySetProxyEnabled) {
  TestCloudPrintProxyService service(&profile_);

  service.GetMockServiceProcessControl()->SetConnectSuccessMockExpectations(
      false);
  service.GetMockCloudPrintProxy().ReturnEnabledInfo();

  sync_preferences::TestingPrefServiceSyncable* prefs =
      profile_.GetTestingPrefService();
  prefs->SetUserPref(prefs::kCloudPrintEmail,
                     base::MakeUnique<base::Value>(std::string()));
  prefs->SetManagedPref(prefs::kCloudPrintProxyEnabled,
                        base::MakeUnique<base::Value>(false));

  service.Initialize();

  EXPECT_EQ(std::string(), prefs->GetString(prefs::kCloudPrintEmail));
  EXPECT_TRUE(service.GetMockCloudPrintProxy().has_been_disabled());
}

TEST_F(CloudPrintProxyPolicyTest, StartWithNoPolicyProxyDisabledThenSetPolicy) {
  TestCloudPrintProxyService service(&profile_);

  service.GetMockServiceProcessControl()->SetConnectSuccessMockExpectations(
      false);
  service.GetMockCloudPrintProxy().ReturnDisabledInfo();

  sync_preferences::TestingPrefServiceSyncable* prefs =
      profile_.GetTestingPrefService();
  prefs->SetUserPref(prefs::kCloudPrintEmail,
                     base::MakeUnique<base::Value>(
                         MockServiceProcessControl::EnabledUserId()));

  service.Initialize();

  EXPECT_EQ(std::string(), prefs->GetString(prefs::kCloudPrintEmail));

  prefs->SetManagedPref(prefs::kCloudPrintProxyEnabled,
                        base::MakeUnique<base::Value>(false));

  EXPECT_EQ(std::string(), prefs->GetString(prefs::kCloudPrintEmail));
}

TEST_F(CloudPrintProxyPolicyTest, StartWithNoPolicyProxyEnabledThenSetPolicy) {
  TestCloudPrintProxyService service(&profile_);

  service.GetMockServiceProcessControl()->SetConnectSuccessMockExpectations(
      false);
  service.GetMockCloudPrintProxy().ReturnEnabledInfo();

  sync_preferences::TestingPrefServiceSyncable* prefs =
      profile_.GetTestingPrefService();
  prefs->SetUserPref(prefs::kCloudPrintEmail,
                     base::MakeUnique<base::Value>(std::string()));

  service.Initialize();
  service.RefreshStatusFromService();

  EXPECT_EQ(MockServiceProcessControl::EnabledUserId(),
            prefs->GetString(prefs::kCloudPrintEmail));

  prefs->SetManagedPref(prefs::kCloudPrintProxyEnabled,
                        base::MakeUnique<base::Value>(false));

  EXPECT_EQ(std::string(), prefs->GetString(prefs::kCloudPrintEmail));
  EXPECT_TRUE(service.GetMockCloudPrintProxy().has_been_disabled());
}

TEST_F(CloudPrintProxyPolicyTest,
       StartWithPolicySetProxyDisabledThenClearPolicy) {
  TestCloudPrintProxyService service(&profile_);

  service.GetMockServiceProcessControl()->SetConnectSuccessMockExpectations(
      false);
  service.GetMockCloudPrintProxy().ReturnDisabledInfo();

  sync_preferences::TestingPrefServiceSyncable* prefs =
      profile_.GetTestingPrefService();
  prefs->SetUserPref(prefs::kCloudPrintEmail,
                     base::MakeUnique<base::Value>(std::string()));
  prefs->SetManagedPref(prefs::kCloudPrintProxyEnabled,
                        base::MakeUnique<base::Value>(false));

  service.Initialize();

  EXPECT_EQ(std::string(), prefs->GetString(prefs::kCloudPrintEmail));
  prefs->RemoveManagedPref(prefs::kCloudPrintProxyEnabled);
  EXPECT_EQ(std::string(), prefs->GetString(prefs::kCloudPrintEmail));
}

TEST_F(CloudPrintProxyPolicyTest,
       StartWithPolicySetProxyEnabledThenClearPolicy) {
  TestCloudPrintProxyService service(&profile_);

  service.GetMockServiceProcessControl()->SetConnectSuccessMockExpectations(
      false);
  service.GetMockCloudPrintProxy().ReturnEnabledInfo();

  sync_preferences::TestingPrefServiceSyncable* prefs =
      profile_.GetTestingPrefService();
  prefs->SetUserPref(prefs::kCloudPrintEmail,
                     base::MakeUnique<base::Value>(std::string()));
  prefs->SetManagedPref(prefs::kCloudPrintProxyEnabled,
                        base::MakeUnique<base::Value>(false));

  service.Initialize();

  EXPECT_EQ(std::string(), prefs->GetString(prefs::kCloudPrintEmail));
  prefs->RemoveManagedPref(prefs::kCloudPrintProxyEnabled);
  EXPECT_EQ(std::string(), prefs->GetString(prefs::kCloudPrintEmail));
  EXPECT_TRUE(service.GetMockCloudPrintProxy().has_been_disabled());
}

TEST_F(CloudPrintProxyPolicyTest, StartWithNoPolicyProxyDisabledThenEnable) {
  TestCloudPrintProxyService service(&profile_);

  service.GetMockServiceProcessControl()->SetConnectSuccessMockExpectations(
      false);
  service.GetMockCloudPrintProxy().ReturnDisabledInfo();

  sync_preferences::TestingPrefServiceSyncable* prefs =
      profile_.GetTestingPrefService();
  prefs->SetUserPref(prefs::kCloudPrintEmail,
                     base::MakeUnique<base::Value>(
                         MockServiceProcessControl::EnabledUserId()));

  service.Initialize();
  EXPECT_EQ(std::string(), prefs->GetString(prefs::kCloudPrintEmail));

  service.EnableForUser();

  EXPECT_EQ(MockServiceProcessControl::EnabledUserId(),
            prefs->GetString(prefs::kCloudPrintEmail));
  EXPECT_TRUE(service.GetMockCloudPrintProxy().has_been_enabled());
}

TEST_F(CloudPrintProxyPolicyTest,
       StartWithPolicySetProxyEnabledThenClearPolicyAndEnable) {
  TestCloudPrintProxyService service(&profile_);

  service.GetMockServiceProcessControl()->SetConnectSuccessMockExpectations(
      false);
  service.GetMockCloudPrintProxy().ReturnEnabledInfo();

  sync_preferences::TestingPrefServiceSyncable* prefs =
      profile_.GetTestingPrefService();
  prefs->SetUserPref(prefs::kCloudPrintEmail,
                     base::MakeUnique<base::Value>(std::string()));
  prefs->SetManagedPref(prefs::kCloudPrintProxyEnabled,
                        base::MakeUnique<base::Value>(false));

  service.Initialize();

  EXPECT_EQ(std::string(), prefs->GetString(prefs::kCloudPrintEmail));
  service.EnableForUser();
  EXPECT_EQ(std::string(), prefs->GetString(prefs::kCloudPrintEmail));

  prefs->RemoveManagedPref(prefs::kCloudPrintProxyEnabled);
  EXPECT_EQ(std::string(), prefs->GetString(prefs::kCloudPrintEmail));

  service.EnableForUser();

  EXPECT_EQ(MockServiceProcessControl::EnabledUserId(),
            prefs->GetString(prefs::kCloudPrintEmail));
  EXPECT_TRUE(service.GetMockCloudPrintProxy().has_been_enabled());
}

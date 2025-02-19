// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/chromedriver/chrome/device_manager.h"

#include <memory>
#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "chrome/test/chromedriver/chrome/adb.h"
#include "chrome/test/chromedriver/chrome/status.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

class FakeAdb : public Adb {
 public:
  FakeAdb() {}
  ~FakeAdb() override {}

  Status GetDevices(std::vector<std::string>* devices) override {
    devices->push_back("a");
    devices->push_back("b");
    return Status(kOk);
  }

  Status ForwardPort(const std::string& device_serial,
                     int local_port,
                     const std::string& remote_abstract) override {
    return Status(kOk);
  }

  Status SetCommandLineFile(const std::string& device_serial,
                            const std::string& command_line_file,
                            const std::string& exec_name,
                            const std::string& args) override {
    return Status(kOk);
  }

  Status CheckAppInstalled(const std::string& device_serial,
                           const std::string& package) override {
    return Status(kOk);
  }

  Status ClearAppData(const std::string& device_serial,
                      const std::string& package) override {
    return Status(kOk);
  }

  Status SetDebugApp(const std::string& device_serial,
                     const std::string& package) override {
    return Status(kOk);
  }

  Status Launch(const std::string& device_serial,
                const std::string& package,
                const std::string& activity) override {
    return Status(kOk);
  }

  Status ForceStop(const std::string& device_serial,
                   const std::string& package) override {
    return Status(kOk);
  }

  Status GetPidByName(const std::string& device_serial,
                      const std::string& process_name,
                      int* pid) override {
    *pid = 0; // avoid uninit error crbug.com/393231
    return Status(kOk);
  }
};

}  // namespace

TEST(DeviceManager, AcquireDevice) {
  FakeAdb adb;
  DeviceManager device_manager(&adb);
  std::unique_ptr<Device> device1;
  std::unique_ptr<Device> device2;
  std::unique_ptr<Device> device3;
  ASSERT_TRUE(device_manager.AcquireDevice(&device1).IsOk());
  ASSERT_TRUE(device_manager.AcquireDevice(&device2).IsOk());
  ASSERT_FALSE(device_manager.AcquireDevice(&device3).IsOk());
  device1.reset(NULL);
  ASSERT_TRUE(device_manager.AcquireDevice(&device3).IsOk());
  ASSERT_FALSE(device_manager.AcquireDevice(&device1).IsOk());
}

TEST(DeviceManager, AcquireSpecificDevice) {
  FakeAdb adb;
  DeviceManager device_manager(&adb);
  std::unique_ptr<Device> device1;
  std::unique_ptr<Device> device2;
  std::unique_ptr<Device> device3;
  ASSERT_TRUE(device_manager.AcquireSpecificDevice("a", &device1).IsOk());
  ASSERT_FALSE(device_manager.AcquireSpecificDevice("a", &device2).IsOk());
  ASSERT_TRUE(device_manager.AcquireSpecificDevice("b", &device3).IsOk());
  device1.reset(NULL);
  ASSERT_TRUE(device_manager.AcquireSpecificDevice("a", &device2).IsOk());
  ASSERT_FALSE(device_manager.AcquireSpecificDevice("a", &device1).IsOk());
  ASSERT_FALSE(device_manager.AcquireSpecificDevice("b", &device1).IsOk());
}

TEST(Device, StartStopApp) {
  FakeAdb adb;
  DeviceManager device_manager(&adb);
  std::unique_ptr<Device> device1;
  ASSERT_TRUE(device_manager.AcquireDevice(&device1).IsOk());
  ASSERT_TRUE(device1->TearDown().IsOk());
  ASSERT_TRUE(device1->SetUp("a.chrome.package", "", "", "", false, 0).IsOk());
  ASSERT_FALSE(device1->SetUp("a.chrome.package", "", "", "", false, 0).IsOk());
  ASSERT_TRUE(device1->TearDown().IsOk());
  ASSERT_FALSE(device1->SetUp(
      "a.chrome.package", "an.activity", "", "", false, 0).IsOk());
  ASSERT_FALSE(device1->SetUp("a.package", "", "", "", false, 0).IsOk());
  ASSERT_TRUE(device1->SetUp(
      "a.package", "an.activity", "", "", false, 0).IsOk());
  ASSERT_TRUE(device1->TearDown().IsOk());
  ASSERT_TRUE(device1->TearDown().IsOk());
  ASSERT_TRUE(device1->SetUp(
      "a.package", "an.activity", "a.process", "", false, 0).IsOk());
  ASSERT_TRUE(device1->TearDown().IsOk());
}

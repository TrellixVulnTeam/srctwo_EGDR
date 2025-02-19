// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/device/public/cpp/power_monitor/power_monitor_broadcast_source.h"

#include "base/macros.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/test/power_monitor_test_base.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace device {

class PowerMonitorBroadcastSourceTest : public testing::Test {
 protected:
  PowerMonitorBroadcastSourceTest() {
    power_monitor_source_ = new PowerMonitorBroadcastSource(nullptr);
    power_monitor_.reset(new base::PowerMonitor(
        std::unique_ptr<base::PowerMonitorSource>(power_monitor_source_)));
  }
  ~PowerMonitorBroadcastSourceTest() override {}

  PowerMonitorBroadcastSource* source() { return power_monitor_source_; }
  base::PowerMonitor* monitor() { return power_monitor_.get(); }

  base::MessageLoop message_loop_;

 private:
  PowerMonitorBroadcastSource* power_monitor_source_;
  std::unique_ptr<base::PowerMonitor> power_monitor_;

  DISALLOW_COPY_AND_ASSIGN(PowerMonitorBroadcastSourceTest);
};

TEST_F(PowerMonitorBroadcastSourceTest, PowerMessageReceiveBroadcast) {
  base::PowerMonitorTestObserver observer;
  monitor()->AddObserver(&observer);

  // Sending resume when not suspended should have no effect.
  source()->Resume();
  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(observer.resumes(), 0);

  // Pretend we suspended.
  source()->Suspend();
  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(observer.suspends(), 1);

  // Send a second suspend notification.  This should be suppressed.
  source()->Suspend();
  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(observer.suspends(), 1);

  // Pretend we were awakened.
  source()->Resume();
  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(observer.resumes(), 1);

  // Send a duplicate resume notification.  This should be suppressed.
  source()->Resume();
  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(observer.resumes(), 1);

  // Pretend the device has gone on battery power
  source()->PowerStateChange(true);
  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(observer.power_state_changes(), 1);
  EXPECT_EQ(observer.last_power_state(), true);

  // Repeated indications the device is on battery power should be suppressed.
  source()->PowerStateChange(true);
  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(observer.power_state_changes(), 1);

  // Pretend the device has gone off battery power
  source()->PowerStateChange(false);
  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(observer.power_state_changes(), 2);
  EXPECT_EQ(observer.last_power_state(), false);

  // Repeated indications the device is off battery power should be suppressed.
  source()->PowerStateChange(false);
  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(observer.power_state_changes(), 2);
}

}  // namespace device

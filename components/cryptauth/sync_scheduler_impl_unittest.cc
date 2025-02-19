// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/cryptauth/sync_scheduler_impl.h"

#include <utility>

#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/timer/mock_timer.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace cryptauth {

using Strategy = SyncScheduler::Strategy;
using SyncState = SyncScheduler::SyncState;

namespace {

// Constants configuring the the scheduler.
const int kElapsedTimeDays = 40;
const int kRefreshPeriodDays = 30;
const int kRecoveryPeriodSeconds = 10;
const double kMaxJitterPercentage = 0.1;
const char kTestSchedulerName[] = "TestSyncSchedulerImpl";

// Returns true if |jittered_time_delta| is within the range of a jittered
// |base_time_delta| with a maximum of |max_jitter_ratio|.
bool IsTimeDeltaWithinJitter(const base::TimeDelta& base_time_delta,
                             const base::TimeDelta& jittered_time_delta,
                             double max_jitter_ratio) {
  if (base_time_delta.is_zero())
    return jittered_time_delta.is_zero();

  base::TimeDelta difference =
      (jittered_time_delta - base_time_delta).magnitude();
  double percentage_of_base =
      difference.InMillisecondsF() / base_time_delta.InMillisecondsF();
  return percentage_of_base < max_jitter_ratio;
}

// Test harness for the SyncSchedulerImpl to create MockTimers.
class TestSyncSchedulerImpl : public SyncSchedulerImpl {
 public:
  TestSyncSchedulerImpl(Delegate* delegate,
                        base::TimeDelta refresh_period,
                        base::TimeDelta recovery_period,
                        double max_jitter_ratio)
      : SyncSchedulerImpl(delegate,
                          refresh_period,
                          recovery_period,
                          max_jitter_ratio,
                          kTestSchedulerName) {}

  ~TestSyncSchedulerImpl() override {}

  base::MockTimer* timer() { return mock_timer_; }

 private:
  std::unique_ptr<base::Timer> CreateTimer() override {
    bool retain_user_task = false;
    bool is_repeating = false;
    mock_timer_ = new base::MockTimer(retain_user_task, is_repeating);
    return base::WrapUnique(mock_timer_);
  }

  // A timer instance for testing. Owned by the parent scheduler.
  base::MockTimer* mock_timer_;

  DISALLOW_COPY_AND_ASSIGN(TestSyncSchedulerImpl);
};

}  // namespace

class CryptAuthSyncSchedulerImplTest : public testing::Test,
                              public SyncSchedulerImpl::Delegate {
 protected:
  CryptAuthSyncSchedulerImplTest()
      : refresh_period_(base::TimeDelta::FromDays(kRefreshPeriodDays)),
        base_recovery_period_(
            base::TimeDelta::FromSeconds(kRecoveryPeriodSeconds)),
        zero_elapsed_time_(base::TimeDelta::FromSeconds(0)),
        scheduler_(new TestSyncSchedulerImpl(this,
                                             refresh_period_,
                                             base_recovery_period_,
                                             0)) {}

  ~CryptAuthSyncSchedulerImplTest() override {}

  void OnSyncRequested(
      std::unique_ptr<SyncScheduler::SyncRequest> sync_request) override {
    sync_request_ = std::move(sync_request);
  }

  base::MockTimer* timer() { return scheduler_->timer(); }

  // The time deltas used to configure |scheduler_|.
  base::TimeDelta refresh_period_;
  base::TimeDelta base_recovery_period_;
  base::TimeDelta zero_elapsed_time_;

  // The scheduler instance under test.
  std::unique_ptr<TestSyncSchedulerImpl> scheduler_;

  std::unique_ptr<SyncScheduler::SyncRequest> sync_request_;

  DISALLOW_COPY_AND_ASSIGN(CryptAuthSyncSchedulerImplTest);
};

TEST_F(CryptAuthSyncSchedulerImplTest, ForceSyncSuccess) {
  scheduler_->Start(zero_elapsed_time_, Strategy::PERIODIC_REFRESH);
  EXPECT_EQ(Strategy::PERIODIC_REFRESH, scheduler_->GetStrategy());
  EXPECT_EQ(SyncState::WAITING_FOR_REFRESH, scheduler_->GetSyncState());

  scheduler_->ForceSync();
  EXPECT_EQ(SyncState::SYNC_IN_PROGRESS, scheduler_->GetSyncState());
  EXPECT_TRUE(sync_request_);
  sync_request_->OnDidComplete(true);
  EXPECT_EQ(Strategy::PERIODIC_REFRESH, scheduler_->GetStrategy());
  EXPECT_EQ(SyncState::WAITING_FOR_REFRESH, scheduler_->GetSyncState());
}

TEST_F(CryptAuthSyncSchedulerImplTest, ForceSyncFailure) {
  scheduler_->Start(zero_elapsed_time_, Strategy::PERIODIC_REFRESH);
  EXPECT_EQ(Strategy::PERIODIC_REFRESH, scheduler_->GetStrategy());

  scheduler_->ForceSync();
  EXPECT_TRUE(sync_request_);
  sync_request_->OnDidComplete(false);
  EXPECT_EQ(Strategy::AGGRESSIVE_RECOVERY, scheduler_->GetStrategy());
}

TEST_F(CryptAuthSyncSchedulerImplTest, PeriodicRefreshSuccess) {
  EXPECT_EQ(SyncState::NOT_STARTED, scheduler_->GetSyncState());
  scheduler_->Start(zero_elapsed_time_, Strategy::PERIODIC_REFRESH);
  EXPECT_EQ(Strategy::PERIODIC_REFRESH, scheduler_->GetStrategy());

  EXPECT_EQ(refresh_period_, timer()->GetCurrentDelay());
  timer()->Fire();
  EXPECT_EQ(SyncState::SYNC_IN_PROGRESS, scheduler_->GetSyncState());
  ASSERT_TRUE(sync_request_.get());

  sync_request_->OnDidComplete(true);
  EXPECT_EQ(SyncState::WAITING_FOR_REFRESH, scheduler_->GetSyncState());
  EXPECT_EQ(Strategy::PERIODIC_REFRESH, scheduler_->GetStrategy());
}

TEST_F(CryptAuthSyncSchedulerImplTest, PeriodicRefreshFailure) {
  scheduler_->Start(zero_elapsed_time_, Strategy::PERIODIC_REFRESH);
  EXPECT_EQ(Strategy::PERIODIC_REFRESH, scheduler_->GetStrategy());
  timer()->Fire();
  sync_request_->OnDidComplete(false);
  EXPECT_EQ(Strategy::AGGRESSIVE_RECOVERY, scheduler_->GetStrategy());
}

TEST_F(CryptAuthSyncSchedulerImplTest, AggressiveRecoverySuccess) {
  scheduler_->Start(zero_elapsed_time_, Strategy::AGGRESSIVE_RECOVERY);
  EXPECT_EQ(Strategy::AGGRESSIVE_RECOVERY, scheduler_->GetStrategy());

  EXPECT_EQ(base_recovery_period_, timer()->GetCurrentDelay());
  timer()->Fire();
  EXPECT_EQ(SyncState::SYNC_IN_PROGRESS, scheduler_->GetSyncState());
  ASSERT_TRUE(sync_request_.get());

  sync_request_->OnDidComplete(true);
  EXPECT_EQ(SyncState::WAITING_FOR_REFRESH, scheduler_->GetSyncState());
  EXPECT_EQ(Strategy::PERIODIC_REFRESH, scheduler_->GetStrategy());
}

TEST_F(CryptAuthSyncSchedulerImplTest, AggressiveRecoveryFailure) {
  scheduler_->Start(zero_elapsed_time_, Strategy::AGGRESSIVE_RECOVERY);

  timer()->Fire();
  sync_request_->OnDidComplete(false);
  EXPECT_EQ(Strategy::AGGRESSIVE_RECOVERY, scheduler_->GetStrategy());
}

TEST_F(CryptAuthSyncSchedulerImplTest, AggressiveRecoveryBackOff) {
  scheduler_->Start(zero_elapsed_time_, Strategy::AGGRESSIVE_RECOVERY);
  base::TimeDelta last_recovery_period = base::TimeDelta::FromSeconds(0);

  for (int i = 0; i < 20; ++i) {
    timer()->Fire();
    EXPECT_EQ(SyncState::SYNC_IN_PROGRESS, scheduler_->GetSyncState());
    sync_request_->OnDidComplete(false);
    EXPECT_EQ(Strategy::AGGRESSIVE_RECOVERY, scheduler_->GetStrategy());
    EXPECT_EQ(SyncState::WAITING_FOR_REFRESH, scheduler_->GetSyncState());

    base::TimeDelta recovery_period = scheduler_->GetTimeToNextSync();
    EXPECT_LE(last_recovery_period, recovery_period);
    last_recovery_period = recovery_period;
  }

  // Backoffs should rapidly converge to the normal refresh period.
  EXPECT_EQ(refresh_period_, last_recovery_period);
}

TEST_F(CryptAuthSyncSchedulerImplTest, RefreshFailureRecoverySuccess) {
  scheduler_->Start(zero_elapsed_time_, Strategy::PERIODIC_REFRESH);
  EXPECT_EQ(Strategy::PERIODIC_REFRESH, scheduler_->GetStrategy());

  timer()->Fire();
  sync_request_->OnDidComplete(false);
  EXPECT_EQ(Strategy::AGGRESSIVE_RECOVERY, scheduler_->GetStrategy());

  timer()->Fire();
  sync_request_->OnDidComplete(true);
  EXPECT_EQ(Strategy::PERIODIC_REFRESH, scheduler_->GetStrategy());
}

TEST_F(CryptAuthSyncSchedulerImplTest, SyncImmediatelyForPeriodicRefresh) {
  scheduler_->Start(base::TimeDelta::FromDays(kElapsedTimeDays),
                    Strategy::PERIODIC_REFRESH);
  EXPECT_TRUE(scheduler_->GetTimeToNextSync().is_zero());
  EXPECT_TRUE(timer()->GetCurrentDelay().is_zero());
  timer()->Fire();
  EXPECT_TRUE(sync_request_);

  EXPECT_EQ(Strategy::PERIODIC_REFRESH, scheduler_->GetStrategy());
}

TEST_F(CryptAuthSyncSchedulerImplTest,
       SyncImmediatelyForAggressiveRecovery) {
  scheduler_->Start(base::TimeDelta::FromDays(kElapsedTimeDays),
                    Strategy::AGGRESSIVE_RECOVERY);
  EXPECT_TRUE(scheduler_->GetTimeToNextSync().is_zero());
  EXPECT_TRUE(timer()->GetCurrentDelay().is_zero());
  timer()->Fire();
  EXPECT_TRUE(sync_request_);

  EXPECT_EQ(Strategy::AGGRESSIVE_RECOVERY, scheduler_->GetStrategy());
}

TEST_F(CryptAuthSyncSchedulerImplTest, InitialSyncShorterByElapsedTime) {
  base::TimeDelta elapsed_time = base::TimeDelta::FromDays(2);
  scheduler_->Start(elapsed_time, Strategy::PERIODIC_REFRESH);
  EXPECT_EQ(refresh_period_ - elapsed_time, scheduler_->GetTimeToNextSync());
  timer()->Fire();
  EXPECT_TRUE(sync_request_);
}

TEST_F(CryptAuthSyncSchedulerImplTest, PeriodicRefreshJitter) {
  scheduler_.reset(new TestSyncSchedulerImpl(
      this, refresh_period_, base_recovery_period_, kMaxJitterPercentage));

  scheduler_->Start(zero_elapsed_time_, Strategy::PERIODIC_REFRESH);

  base::TimeDelta cumulative_jitter = base::TimeDelta::FromSeconds(0);
  for (int i = 0; i < 10; ++i) {
    base::TimeDelta next_sync_delta = scheduler_->GetTimeToNextSync();
    cumulative_jitter += (next_sync_delta - refresh_period_).magnitude();
    EXPECT_TRUE(IsTimeDeltaWithinJitter(refresh_period_, next_sync_delta,
                                        kMaxJitterPercentage));
    timer()->Fire();
    sync_request_->OnDidComplete(true);
  }

  // The probablility that all periods are randomly equal to |refresh_period_|
  // is so low that we would expect the heat death of the universe before this
  // test flakes.
  EXPECT_FALSE(cumulative_jitter.is_zero());
}

TEST_F(CryptAuthSyncSchedulerImplTest, JitteredTimeDeltaIsNonNegative) {
  base::TimeDelta zero_delta = base::TimeDelta::FromSeconds(0);
  double max_jitter_ratio = 1;
  scheduler_.reset(new TestSyncSchedulerImpl(this, zero_delta, zero_delta,
                                             max_jitter_ratio));
  scheduler_->Start(zero_elapsed_time_, Strategy::PERIODIC_REFRESH);

  for (int i = 0; i < 10; ++i) {
    base::TimeDelta next_sync_delta = scheduler_->GetTimeToNextSync();
    EXPECT_GE(zero_delta, next_sync_delta);
    EXPECT_TRUE(
        IsTimeDeltaWithinJitter(zero_delta, next_sync_delta, max_jitter_ratio));
    timer()->Fire();
    sync_request_->OnDidComplete(true);
  }
}

TEST_F(CryptAuthSyncSchedulerImplTest, StartWithNegativeElapsedTime) {
  // This could happen in rare cases where the system clock changes.
  scheduler_->Start(base::TimeDelta::FromDays(-1000),
                    Strategy::PERIODIC_REFRESH);

  base::TimeDelta zero_delta = base::TimeDelta::FromSeconds(0);
  EXPECT_EQ(zero_delta, scheduler_->GetTimeToNextSync());
  EXPECT_EQ(zero_delta, timer()->GetCurrentDelay());
}

}  // namespace cryptauth

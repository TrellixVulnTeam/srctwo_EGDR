// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromecast/base/system_time_change_notifier.h"

#include <memory>
#include <utility>

#include "base/bind.h"
#include "base/location.h"
#include "base/macros.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/sequenced_task_runner.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/time/time.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace chromecast {

namespace {

class SequencedTaskRunnerNoDelay : public base::SequencedTaskRunner {
 public:
  SequencedTaskRunnerNoDelay() {}

  // base::SequencedTaskRunner implementation:
  bool PostDelayedTask(const tracked_objects::Location& from_here,
                       base::OnceClosure task,
                       base::TimeDelta delay) override {
    base::ThreadTaskRunnerHandle::Get()->PostTask(from_here, std::move(task));
    return true;
  }

  bool PostNonNestableDelayedTask(const tracked_objects::Location& from_here,
                                  base::OnceClosure task,
                                  base::TimeDelta delay) override {
    return true;
  }

  bool RunsTasksInCurrentSequence() const override { return true; }

 private:
  ~SequencedTaskRunnerNoDelay() override {}

  DISALLOW_COPY_AND_ASSIGN(SequencedTaskRunnerNoDelay);
};

class TimeChangeObserver : public SystemTimeChangeNotifier::Observer {
 public:
  TimeChangeObserver() : num_time_changed_(0) {}
  ~TimeChangeObserver() override {}

  // SystemTimeChangeNotifier::Observer implementation:
  void OnSystemTimeChanged() override { ++num_time_changed_; }

  int num_time_changed() const { return num_time_changed_; }

 private:
  int num_time_changed_;

  DISALLOW_COPY_AND_ASSIGN(TimeChangeObserver);
};

}  // namespace

class SystemTimeChangeNotifierTest : public testing::Test {
 protected:
  void SetUp() override {
    message_loop_.reset(new base::MessageLoop);

    notifier_.reset(new SystemTimeChangeNotifierPeriodicMonitor(
        new SequencedTaskRunnerNoDelay()));

    observer_.reset(new TimeChangeObserver);
    notifier_->AddObserver(observer_.get());
    notifier_->Initialize();
  }

  void TearDown() override {
    notifier_->Finalize();
  }

  // Runs pending tasks. It doesn't run tasks schedule after this call.
  void RunPendingTasks() {
    base::RunLoop run_loop;
    base::ThreadTaskRunnerHandle::Get()->PostTask(FROM_HERE,
                                                  run_loop.QuitClosure());
    run_loop.Run();
  }

  std::unique_ptr<base::MessageLoop> message_loop_;
  std::unique_ptr<SystemTimeChangeNotifierPeriodicMonitor> notifier_;
  std::unique_ptr<TimeChangeObserver> observer_;
};

TEST_F(SystemTimeChangeNotifierTest, NotChanged) {
  EXPECT_EQ(0, observer_->num_time_changed());

  // Not changed.
  RunPendingTasks();
  EXPECT_EQ(0, observer_->num_time_changed());
}

TEST_F(SystemTimeChangeNotifierTest, TimeChangedForwardLessThan10Seconds) {
  base::Time now = base::Time::Now();
  EXPECT_EQ(0, observer_->num_time_changed());

  // Time change NOT detected.
  notifier_->set_fake_now_for_testing(now + base::TimeDelta::FromSeconds(4));
  RunPendingTasks();
  EXPECT_EQ(0, observer_->num_time_changed());
  RunPendingTasks();
  EXPECT_EQ(0, observer_->num_time_changed());
}

TEST_F(SystemTimeChangeNotifierTest, TimeChangedBackwardLessThan10Seconds) {
  base::Time now = base::Time::Now();
  EXPECT_EQ(0, observer_->num_time_changed());

  // Time change NOT detected.
  notifier_->set_fake_now_for_testing(now - base::TimeDelta::FromSeconds(4));
  RunPendingTasks();
  EXPECT_EQ(0, observer_->num_time_changed());
  RunPendingTasks();
  EXPECT_EQ(0, observer_->num_time_changed());
}

TEST_F(SystemTimeChangeNotifierTest, TimeChangedForwardMoreThan10Seconds) {
  base::Time now = base::Time::Now();
  EXPECT_EQ(0, observer_->num_time_changed());

  notifier_->set_fake_now_for_testing(now + base::TimeDelta::FromSeconds(40));
  RunPendingTasks();
  // Still 0 since observe callback is running in next run loop.
  EXPECT_EQ(0, observer_->num_time_changed());
  RunPendingTasks();
  // Time change detected.
  EXPECT_EQ(1, observer_->num_time_changed());
}

TEST_F(SystemTimeChangeNotifierTest, TimeChangedBackwardMoreThan10Seconds) {
  base::Time now = base::Time::Now();
  EXPECT_EQ(0, observer_->num_time_changed());

  notifier_->set_fake_now_for_testing(now - base::TimeDelta::FromSeconds(40));
  RunPendingTasks();
  // Still 0 since observe callback is running in next run loop.
  EXPECT_EQ(0, observer_->num_time_changed());
  RunPendingTasks();
  // Time change detected.
  EXPECT_EQ(1, observer_->num_time_changed());
}

TEST_F(SystemTimeChangeNotifierTest, CannotDetectTimeDriftForward) {
  base::Time now = base::Time::Now();
  EXPECT_EQ(0, observer_->num_time_changed());

  // Time change NOT detected. Expected = now + 1, actual = now + 4.
  notifier_->set_fake_now_for_testing(now + base::TimeDelta::FromSeconds(4));
  RunPendingTasks();
  EXPECT_EQ(0, observer_->num_time_changed());

  // Time change NOT detected. Expected = now + 4 + 1, actual = now + 8.
  notifier_->set_fake_now_for_testing(now + base::TimeDelta::FromSeconds(8));
  RunPendingTasks();
  EXPECT_EQ(0, observer_->num_time_changed());

  // Time change NOT detected. Expected = now + 8 + 1, actual = now + 12.
  notifier_->set_fake_now_for_testing(now + base::TimeDelta::FromSeconds(12));
  RunPendingTasks();
  EXPECT_EQ(0, observer_->num_time_changed());

  // Time change detected. Expected = now + 12 + 1, actual = now + 16.
  notifier_->set_fake_now_for_testing(now + base::TimeDelta::FromSeconds(16));
  RunPendingTasks();
  EXPECT_EQ(0, observer_->num_time_changed());

  // Time change detected. Expected = now + 16 + 1, actual = now + 20.
  notifier_->set_fake_now_for_testing(now + base::TimeDelta::FromSeconds(20));
  RunPendingTasks();
  EXPECT_EQ(0, observer_->num_time_changed());
}

TEST_F(SystemTimeChangeNotifierTest, CannotDetectTTimeDriftBackward) {
  base::Time now = base::Time::Now();
  EXPECT_EQ(0, observer_->num_time_changed());

  // Time change NOT detected. Expected = now + 1, actual = now - 4.
  notifier_->set_fake_now_for_testing(now - base::TimeDelta::FromSeconds(4));
  RunPendingTasks();
  EXPECT_EQ(0, observer_->num_time_changed());

  // Time change NOT detected. Expected = now - 4 + 1, actual = now - 8.
  notifier_->set_fake_now_for_testing(now - base::TimeDelta::FromSeconds(8));
  RunPendingTasks();
  EXPECT_EQ(0, observer_->num_time_changed());

  // Time change detected. Expected = now - 8 + 1, actual = now - 12.
  notifier_->set_fake_now_for_testing(now - base::TimeDelta::FromSeconds(12));
  RunPendingTasks();
  EXPECT_EQ(0, observer_->num_time_changed());

  // Time change detected. Expected = now - 12 + 1, actual = now - 16.
  notifier_->set_fake_now_for_testing(now - base::TimeDelta::FromSeconds(16));
  RunPendingTasks();
  EXPECT_EQ(0, observer_->num_time_changed());

  // Time change detected. Expected = now - 20 + 1, actual = now - 20.
  notifier_->set_fake_now_for_testing(now - base::TimeDelta::FromSeconds(20));
  RunPendingTasks();
  EXPECT_EQ(0, observer_->num_time_changed());
}

}  // namespace chromecast

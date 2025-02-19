// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "base/message_loop/message_loop.h"
#include "base/metrics/user_metrics.h"
#include "base/metrics/user_metrics_action.h"
#include "base/run_loop.h"
#include "base/stl_util.h"
#include "base/test/histogram_tester.h"
#include "base/time/time.h"
#include "ios/chrome/browser/metrics/first_user_action_recorder.h"
#include "ios/chrome/browser/ui/ui_util.h"
#include "ios/web/public/test/test_web_thread.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/platform_test.h"

using base::UserMetricsAction;

class FirstUserActionRecorderTest : public PlatformTest {
 protected:
  void SetUp() override {
    loop_.reset(new base::MessageLoop(base::MessageLoop::TYPE_DEFAULT));
    ui_thread_.reset(new web::TestWebThread(web::WebThread::UI, loop_.get()));

    base::TimeDelta delta = base::TimeDelta::FromSeconds(60);
    recorder_.reset(new FirstUserActionRecorder(delta));

    histogram_tester_.reset(new base::HistogramTester());

    is_pad_ = IsIPadIdiom();
  }

  bool is_pad_;

  std::unique_ptr<base::MessageLoop> loop_;
  std::unique_ptr<web::TestWebThread> ui_thread_;

  std::unique_ptr<FirstUserActionRecorder> recorder_;
  std::unique_ptr<base::HistogramTester> histogram_tester_;
};

TEST_F(FirstUserActionRecorderTest, Expire) {
  recorder_->Expire();

  // Verify the first user action histogram contains the single expiration
  // value.
  histogram_tester_->ExpectUniqueSample(
      kFirstUserActionTypeHistogramName[is_pad_],
      FirstUserActionRecorder::EXPIRATION, 1);

  // Verify the expiration histogram contains a single duration value.
  // TODO(crbug.com/546363): Ideally this would also verify the value is in the
  // correct bucket.
  histogram_tester_->ExpectTotalCount(
      kFirstUserActionExpirationHistogramName[is_pad_], 1);
}

TEST_F(FirstUserActionRecorderTest, RecordStartOnNTP) {
  recorder_->RecordStartOnNTP();

  // Verify the first user action histogram contains the single start on NTP
  // value.
  histogram_tester_->ExpectUniqueSample(
      kFirstUserActionTypeHistogramName[is_pad_],
      FirstUserActionRecorder::START_ON_NTP, 1);
}

TEST_F(FirstUserActionRecorderTest, OnUserAction_Continuation) {
  base::RecordAction(UserMetricsAction("MobileContextMenuOpenLink"));

  // Verify the first user action histogram contains the single continuation
  // value.
  histogram_tester_->ExpectUniqueSample(
      kFirstUserActionTypeHistogramName[is_pad_],
      FirstUserActionRecorder::CONTINUATION, 1);

  // Verify the continuation histogram contains a single duration value.
  // TODO(crbug.com/546363): Ideally this would also verify the value is in the
  // correct bucket.
  histogram_tester_->ExpectTotalCount(
      kFirstUserActionContinuationHistogramName[is_pad_], 1);
}

TEST_F(FirstUserActionRecorderTest, OnUserAction_NewTask) {
  base::RecordAction(UserMetricsAction("MobileMenuNewTab"));

  // Verify the first user action histogram contains the single 'new task'
  // value.
  histogram_tester_->ExpectUniqueSample(
      kFirstUserActionTypeHistogramName[is_pad_],
      FirstUserActionRecorder::NEW_TASK, 1);

  // Verify the 'new task' histogram contains a single duration value.
  // TODO(crbug.com/546363): Ideally this would also verify the value is in the
  // correct bucket.
  histogram_tester_->ExpectTotalCount(
      kFirstUserActionNewTaskHistogramName[is_pad_], 1);
}

TEST_F(FirstUserActionRecorderTest, OnUserAction_Ignored) {
  base::RecordAction(UserMetricsAction("MobileTabClosed"));

  // Verify the first user action histogram contains no values.
  histogram_tester_->ExpectTotalCount(
      kFirstUserActionTypeHistogramName[is_pad_], 0);

  // Verify the duration histograms contain no values.
  histogram_tester_->ExpectTotalCount(
      kFirstUserActionNewTaskHistogramName[is_pad_], 0);
  histogram_tester_->ExpectTotalCount(
      kFirstUserActionContinuationHistogramName[is_pad_], 0);
  histogram_tester_->ExpectTotalCount(
      kFirstUserActionExpirationHistogramName[is_pad_], 0);
}

TEST_F(FirstUserActionRecorderTest, OnUserAction_RethrowAction_Continuation) {
  base::RecordAction(UserMetricsAction("MobileTabSwitched"));
  base::RunLoop().RunUntilIdle();

  // Verify the first user action histogram contains the single continuation
  // value.
  histogram_tester_->ExpectUniqueSample(
      kFirstUserActionTypeHistogramName[is_pad_],
      FirstUserActionRecorder::CONTINUATION, 1);

  // Verify the continuation histogram contains a single duration value.
  // TODO(crbug.com/546363): Ideally this would also verify the value is in the
  // correct bucket.
  histogram_tester_->ExpectTotalCount(
      kFirstUserActionContinuationHistogramName[is_pad_], 1);
}

TEST_F(FirstUserActionRecorderTest, OnUserAction_RethrowAction_NewTask) {
  base::RecordAction(UserMetricsAction("MobileTabSwitched"));
  base::RecordAction(UserMetricsAction("MobileTabStripNewTab"));

  // Verify the first user action histogram contains the single 'new task'
  // value.
  histogram_tester_->ExpectUniqueSample(
      kFirstUserActionTypeHistogramName[is_pad_],
      FirstUserActionRecorder::NEW_TASK, 1);

  // Verify the 'new task' histogram contains the a single duration value.
  // TODO(crbug.com/546363): Ideally this would also verify the value is in the
  // correct bucket.
  histogram_tester_->ExpectTotalCount(
      kFirstUserActionNewTaskHistogramName[is_pad_], 1);
}

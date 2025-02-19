// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/metrics/desktop_session_duration/audible_contents_tracker.h"

#include "base/path_service.h"
#include "base/run_loop.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "content/public/test/browser_test_base.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

// Observer for testing AudibleContentsTracker.
class MockAudibleContentsObserver
    : public metrics::AudibleContentsTracker::Observer {
 public:
  MockAudibleContentsObserver() {}

  // AudibleContentsTracker::Observer:
  void OnAudioStart() override { is_audio_playing_ = true; }
  void OnAudioEnd() override { is_audio_playing_ = false; }

  bool is_audio_playing() const { return is_audio_playing_; }

 private:
  bool is_audio_playing_ = false;

  DISALLOW_COPY_AND_ASSIGN(MockAudibleContentsObserver);
};

}  // namespace

class AudibleContentsTrackerTest : public InProcessBrowserTest {
 public:
  AudibleContentsTrackerTest() {}

  void SetUp() override {
    observer_.reset(new MockAudibleContentsObserver());
    tracker_.reset(new metrics::AudibleContentsTracker(observer()));
    InProcessBrowserTest::SetUp();
  }

  void TearDown() override {
    InProcessBrowserTest::TearDown();
    tracker_.reset();
    observer_.reset();
  }

  MockAudibleContentsObserver* observer() const { return observer_.get(); }

 private:
  std::unique_ptr<MockAudibleContentsObserver> observer_ = nullptr;
  std::unique_ptr<metrics::AudibleContentsTracker> tracker_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(AudibleContentsTrackerTest);
};

IN_PROC_BROWSER_TEST_F(AudibleContentsTrackerTest, TestAudioNotifications) {
  MockAudibleContentsObserver* audio_observer = observer();
  EXPECT_FALSE(audio_observer->is_audio_playing());

  // Add a request handler for serving audio.
  base::FilePath test_data_dir;
  ASSERT_TRUE(PathService::Get(base::DIR_SOURCE_ROOT, &test_data_dir));
  embedded_test_server()->ServeFilesFromDirectory(
      test_data_dir.AppendASCII("chrome/test/data/"));
  // Start the test server after adding the request handler for thread safety.
  ASSERT_TRUE(embedded_test_server()->Start());
  ui_test_utils::NavigateToURL(
      browser(), embedded_test_server()->GetURL("/autoplay_audio.html"));

  // Wait until the audio starts.
  while (!audio_observer->is_audio_playing()) {
    base::RunLoop().RunUntilIdle();
  }

  // Wait until the audio stops.
  while (audio_observer->is_audio_playing()) {
    base::RunLoop().RunUntilIdle();
  }
}

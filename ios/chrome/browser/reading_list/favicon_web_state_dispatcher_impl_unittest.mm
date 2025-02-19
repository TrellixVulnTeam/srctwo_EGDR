// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/reading_list/favicon_web_state_dispatcher_impl.h"

#include "base/memory/ptr_util.h"
#include "components/favicon/ios/web_favicon_driver.h"
#include "ios/chrome/browser/browser_state/test_chrome_browser_state.h"
#import "ios/testing/wait_util.h"
#include "ios/web/public/test/test_web_thread_bundle.h"
#include "ios/web/public/web_state/web_state_observer.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/platform_test.h"

namespace reading_list {

// Test class.
class FaviconWebStateDispatcherTest : public PlatformTest {
 public:
  FaviconWebStateDispatcherTest() : web_state_destroyed_(false) {
    TestChromeBrowserState::Builder builder;
    browser_state_ = builder.Build();
  }

  web::BrowserState* GetBrowserState() { return browser_state_.get(); }

  bool IsWebStateDestroyed() { return web_state_destroyed_; }
  void WebStateDestroyed() { web_state_destroyed_ = true; }

 private:
  web::TestWebThreadBundle thread_bundle_;
  std::unique_ptr<TestChromeBrowserState> browser_state_;
  bool web_state_destroyed_;
};

// Observer for the test.
class TestFaviconWebStateDispatcherObserver : public web::WebStateObserver {
 public:
  TestFaviconWebStateDispatcherObserver(web::WebState* web_state,
                                        FaviconWebStateDispatcherTest* owner)
      : web::WebStateObserver(web_state), owner_(owner) {}

  // WebStateObserver implementation:
  void WebStateDestroyed() override { owner_->WebStateDestroyed(); };

 private:
  FaviconWebStateDispatcherTest* owner_;  // weak, owns this object.
};

// Tests that RequestWebState returns a WebState with a FaviconDriver attached.
TEST_F(FaviconWebStateDispatcherTest, RequestWebState) {
  FaviconWebStateDispatcherImpl dispatcher(GetBrowserState(), -1);
  std::unique_ptr<web::WebState> web_state = dispatcher.RequestWebState();

  favicon::WebFaviconDriver* driver =
      favicon::WebFaviconDriver::FromWebState(web_state.get());
  EXPECT_NE(driver, nullptr);
}

// Tests that the WebState returned will be destroyed after a delay.
TEST_F(FaviconWebStateDispatcherTest, ReturnWebState) {
  FaviconWebStateDispatcherImpl dispatcher(GetBrowserState(), 0);
  std::unique_ptr<web::WebState> web_state = dispatcher.RequestWebState();

  TestFaviconWebStateDispatcherObserver observer(web_state.get(), this);

  ConditionBlock condition = ^{
    return IsWebStateDestroyed();
  };

  dispatcher.ReturnWebState(std::move(web_state));

  ASSERT_TRUE(testing::WaitUntilConditionOrTimeout(0.5, condition));
}
}  // namespace reading_list

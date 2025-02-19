// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <UIKit/UIKit.h>

#include <memory>

#include "base/memory/ptr_util.h"
#include "base/strings/sys_string_conversions.h"
#include "base/version.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/sync_preferences/pref_service_mock_factory.h"
#include "components/sync_preferences/pref_service_syncable.h"
#include "ios/chrome/browser/browser_state/test_chrome_browser_state.h"
#include "ios/chrome/browser/prefs/browser_prefs.h"
#include "ios/chrome/browser/signin/authentication_service_factory.h"
#import "ios/chrome/browser/signin/authentication_service_fake.h"
#import "ios/chrome/browser/ui/promos/signin_promo_view_controller.h"
#include "ios/chrome/test/block_cleanup_test.h"
#import "ios/public/provider/chrome/browser/signin/fake_chrome_identity_service.h"
#include "ios/web/public/test/test_web_thread_bundle.h"
#include "testing/gtest/include/gtest/gtest.h"
#import "third_party/ocmock/OCMock/OCMock.h"
#import "third_party/ocmock/gtest_support.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

@interface SigninPromoViewController (ExposedForTesting)
@property(nonatomic, readonly) UIView* accountsContainer;
- (void)recordPromoDisplayed;
+ (base::Version)currentVersion;
@end

namespace {
// Fake version used in tests.
base::Version* gFakeCurrentVersion = nullptr;
}

@interface FakeVersionSigninPromoViewController : SigninPromoViewController
+ (void)setFakeCurrentVersion:(const std::string&)fakeCurrentVersion;
+ (void)clearFakeCurrentVersion;
@end

@implementation FakeVersionSigninPromoViewController
+ (base::Version)currentVersion {
  DCHECK(gFakeCurrentVersion);
  return *gFakeCurrentVersion;
}

+ (void)setFakeCurrentVersion:(const std::string&)fakeCurrentVersion {
  [self clearFakeCurrentVersion];
  gFakeCurrentVersion = new base::Version(fakeCurrentVersion);
}

+ (void)clearFakeCurrentVersion {
  if (gFakeCurrentVersion) {
    delete gFakeCurrentVersion;
    gFakeCurrentVersion = nullptr;
  }
}
@end

namespace {

enum ButtonTestingID {
  // IBUITag specified in the .xib file.
  FRONT_CLOSE_BUTTON_ID = 202,
};

class SigninPromoViewControllerTest : public BlockCleanupTest {
 public:
  ~SigninPromoViewControllerTest() override {}

  SigninPromoViewController* controller() { return controller_; }

  void SetUp() override {
    BlockCleanupTest::SetUp();
    [FakeVersionSigninPromoViewController setFakeCurrentVersion:"1.0"];
    TestChromeBrowserState::Builder builder;
    builder.SetPrefService(CreatePrefService());
    builder.AddTestingFactory(
        AuthenticationServiceFactory::GetInstance(),
        AuthenticationServiceFake::CreateAuthenticationService);
    chrome_browser_state_ = builder.Build();
    ios::FakeChromeIdentityService::GetInstanceFromChromeProvider()
        ->AddIdentities(@[ @"foo", @"bar" ]);
    controller_ = [[FakeVersionSigninPromoViewController alloc]
        initWithBrowserState:chrome_browser_state_.get()
                  dispatcher:nil];
  }

  void TearDown() override {
    NSUserDefaults* standardDefaults = [NSUserDefaults standardUserDefaults];
    [standardDefaults removeObjectForKey:kDisplayedSSORecallForMajorVersionKey];
    [standardDefaults synchronize];
    [FakeVersionSigninPromoViewController clearFakeCurrentVersion];
    BlockCleanupTest::TearDown();
  }

  std::unique_ptr<sync_preferences::PrefServiceSyncable> CreatePrefService() {
    sync_preferences::PrefServiceMockFactory factory;
    scoped_refptr<user_prefs::PrefRegistrySyncable> registry(
        new user_prefs::PrefRegistrySyncable);
    std::unique_ptr<sync_preferences::PrefServiceSyncable> prefs =
        factory.CreateSyncable(registry.get());
    RegisterBrowserStatePrefs(registry.get());
    return prefs;
  }

  void SetPromoHasBeenShown() {
    SigninPromoViewController* signin_promo_view_controller =
        static_cast<SigninPromoViewController*>(controller());
    EXPECT_TRUE([signin_promo_view_controller
        isKindOfClass:[SigninPromoViewController class]]);
    [signin_promo_view_controller recordPromoDisplayed];
  }

 protected:
  web::TestWebThreadBundle thread_bundle_;
  std::unique_ptr<TestChromeBrowserState> chrome_browser_state_;
  SigninPromoViewController* controller_;
};

TEST_F(SigninPromoViewControllerTest, TestConstructorDestructor) {
  EXPECT_TRUE(controller());
  EXPECT_TRUE([controller() view]);
}

TEST_F(SigninPromoViewControllerTest, TestWillDisplay) {
  EXPECT_TRUE([SigninPromoViewController
      shouldBePresentedForBrowserState:chrome_browser_state_.get()]);
}

TEST_F(SigninPromoViewControllerTest, TestWillNotDisplaySameVersion) {
  SetPromoHasBeenShown();
  EXPECT_FALSE([FakeVersionSigninPromoViewController
      shouldBePresentedForBrowserState:chrome_browser_state_.get()]);
}

TEST_F(SigninPromoViewControllerTest, TestWillNotDisplayOneMinorVersion) {
  SetPromoHasBeenShown();
  // Set the future version to be one minor release ahead.
  [FakeVersionSigninPromoViewController setFakeCurrentVersion:"1.1"];
  EXPECT_FALSE([FakeVersionSigninPromoViewController
      shouldBePresentedForBrowserState:chrome_browser_state_.get()]);
}

TEST_F(SigninPromoViewControllerTest, TestWillNotDisplayTwoMinorVersions) {
  SetPromoHasBeenShown();
  // Set the future version to be two minor releases ahead.
  [FakeVersionSigninPromoViewController setFakeCurrentVersion:"1.2"];
  EXPECT_FALSE([FakeVersionSigninPromoViewController
      shouldBePresentedForBrowserState:chrome_browser_state_.get()]);
}

TEST_F(SigninPromoViewControllerTest, TestWillNotDisplayOneMajorVersion) {
  SetPromoHasBeenShown();
  // Set the future version to be one major release ahead.
  [FakeVersionSigninPromoViewController setFakeCurrentVersion:"2.0"];
  EXPECT_FALSE([FakeVersionSigninPromoViewController
      shouldBePresentedForBrowserState:chrome_browser_state_.get()]);
}

TEST_F(SigninPromoViewControllerTest, TestWillDisplayTwoMajorVersions) {
  SetPromoHasBeenShown();
  // Set the future version to be two major releases ahead.
  [FakeVersionSigninPromoViewController setFakeCurrentVersion:"3.0"];
  EXPECT_TRUE([FakeVersionSigninPromoViewController
      shouldBePresentedForBrowserState:chrome_browser_state_.get()]);
}

}  // namespace

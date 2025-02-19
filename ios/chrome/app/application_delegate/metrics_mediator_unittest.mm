// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/app/application_delegate/metrics_mediator.h"
#import "ios/chrome/app/application_delegate/metrics_mediator_testing.h"

#import <Foundation/Foundation.h>

#include "base/mac/scoped_block.h"
#import "breakpad/src/client/ios/BreakpadController.h"
#include "components/metrics/metrics_service.h"
#import "ios/chrome/app/application_delegate/startup_information.h"
#include "ios/chrome/browser/application_context.h"
#import "ios/chrome/browser/metrics/previous_session_info.h"
#import "ios/chrome/browser/metrics/previous_session_info_private.h"
#import "ios/chrome/browser/tabs/tab_model.h"
#import "ios/chrome/browser/ui/main/browser_view_information.h"
#import "ios/chrome/test/base/scoped_block_swizzler.h"
#import "ios/chrome/test/ocmock/OCMockObject+BreakpadControllerTesting.h"
#include "net/base/network_change_notifier.h"
#include "testing/platform_test.h"
#import "third_party/ocmock/OCMock/OCMock.h"
#include "third_party/ocmock/gtest_support.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

#pragma mark - connectionTypeChanged tests.

// Mock class for testing MetricsMediator.
@interface MetricsMediatorMock : MetricsMediator
@property(nonatomic) NSInteger reportingValue;
@property(nonatomic) NSInteger breakpadUpload;
- (void)reset;
- (void)setReporting:(BOOL)enableReporting;
@end

@implementation MetricsMediatorMock
@synthesize reportingValue = _reportingValue;
@synthesize breakpadUpload = _breakpadUpload;

- (void)reset {
  _reportingValue = -1;
  _breakpadUpload = -1;
}
- (void)setBreakpadUploadingEnabled:(BOOL)enableUploading {
  _breakpadUpload = enableUploading ? 1 : 0;
}
- (void)setReporting:(BOOL)enableReporting {
  _reportingValue = enableReporting ? 1 : 0;
}
- (BOOL)isMetricsReportingEnabledWifiOnly {
  return YES;
}
- (BOOL)areMetricsEnabled {
  return YES;
}

@end

// Gives the differents net::NetworkChangeNotifier::ConnectionType based on
// scenario number.
net::NetworkChangeNotifier::ConnectionType getConnectionType(int number) {
  switch (number) {
    case 0:
      return net::NetworkChangeNotifier::CONNECTION_UNKNOWN;
    case 1:
      return net::NetworkChangeNotifier::CONNECTION_ETHERNET;
    case 2:
      return net::NetworkChangeNotifier::CONNECTION_WIFI;
    case 3:
      return net::NetworkChangeNotifier::CONNECTION_2G;
    case 4:
      return net::NetworkChangeNotifier::CONNECTION_3G;
    case 5:
      return net::NetworkChangeNotifier::CONNECTION_4G;
    case 6:
      return net::NetworkChangeNotifier::CONNECTION_NONE;
    case 7:
      return net::NetworkChangeNotifier::CONNECTION_BLUETOOTH;
    default:
      return net::NetworkChangeNotifier::CONNECTION_UNKNOWN;
  }
}

// Gives the differents expected value based on scenario number.
int getExpectedValue(int number) {
  if (number > 2 && number < 6)
    return 0;
  return 1;
}

// Verifies that connectionTypeChanged correctly enables or disables the
// uploading in the breakpad and in the metrics service.
TEST(MetricsMediatorTest, connectionTypeChanged) {
  [[PreviousSessionInfo sharedInstance] setIsFirstSessionAfterUpgrade:NO];
  MetricsMediatorMock* mock_metrics_helper = [[MetricsMediatorMock alloc] init];

  // Checks all different scenarios.
  for (int i = 0; i < 8; ++i) {
    [mock_metrics_helper reset];
    [mock_metrics_helper connectionTypeChanged:getConnectionType(i)];
    EXPECT_EQ(getExpectedValue(i), [mock_metrics_helper reportingValue]);
    EXPECT_EQ(getExpectedValue(i), [mock_metrics_helper breakpadUpload]);
  }

  // Checks that no new ConnectionType has been added.
  EXPECT_EQ(net::NetworkChangeNotifier::CONNECTION_BLUETOOTH,
            net::NetworkChangeNotifier::CONNECTION_LAST);
}

#pragma mark - logLaunchMetrics tests.

// A block that takes as arguments the caller and the arguments from
// UserActivityHandler +handleStartupParameters and returns nothing.
typedef void (^logLaunchMetricsBlock)(id, const char*, int);

class MetricsMediatorLogLaunchTest : public PlatformTest {
 protected:
  MetricsMediatorLogLaunchTest() : has_been_called_(FALSE) {}

  void initiateMetricsMediator(BOOL coldStart, int tabCount) {
    id mainTabModel = [OCMockObject mockForClass:[TabModel class]];
    [[[mainTabModel stub] andReturnValue:@(tabCount)] count];
    browser_view_information_ =
        [OCMockObject mockForProtocol:@protocol(BrowserViewInformation)];
    [[[browser_view_information_ stub] andReturn:mainTabModel] mainTabModel];

    swizzle_block_ = [^(id self, int numTab) {
      has_been_called_ = YES;
      // Tests.
      EXPECT_EQ(tabCount, numTab);
    } copy];
    if (coldStart) {
      uma_histogram_swizzler_.reset(new ScopedBlockSwizzler(
          [MetricsMediator class], @selector(recordNumTabAtStartup:),
          swizzle_block_));
    } else {
      uma_histogram_swizzler_.reset(new ScopedBlockSwizzler(
          [MetricsMediator class], @selector(recordNumTabAtResume:),
          swizzle_block_));
    }
  }

  void verifySwizzleHasBeenCalled() { EXPECT_TRUE(has_been_called_); }

  id getBrowserViewInformation() { return browser_view_information_; }

 private:
  id browser_view_information_;
  __block BOOL has_been_called_;
  logLaunchMetricsBlock swizzle_block_;
  std::unique_ptr<ScopedBlockSwizzler> uma_histogram_swizzler_;
};

// Verifies that the log of the number of open tabs is sent and verifies
TEST_F(MetricsMediatorLogLaunchTest,
       logLaunchMetricsWithEnteredBackgroundDate) {
  // Setup.
  BOOL coldStart = YES;
  initiateMetricsMediator(coldStart, 23);

  const NSTimeInterval kFirstUserActionTimeout = 30.0;

  id startupInformation =
      [OCMockObject niceMockForProtocol:@protocol(StartupInformation)];
  [[[startupInformation stub] andReturnValue:@(coldStart)] isColdStart];
  [[startupInformation expect]
      expireFirstUserActionRecorderAfterDelay:kFirstUserActionTimeout];

  id currentTabModel = [OCMockObject mockForClass:[TabModel class]];
  [[[currentTabModel stub] andReturn:nil] currentTab];

  id browserViewInformation = getBrowserViewInformation();
  [[[browserViewInformation stub] andReturn:currentTabModel] currentTabModel];

  [[NSUserDefaults standardUserDefaults]
      setObject:[NSDate date]
         forKey:metrics_mediator::kAppEnteredBackgroundDateKey];

  // Action.
  [MetricsMediator
      logLaunchMetricsWithStartupInformation:startupInformation
                      browserViewInformation:getBrowserViewInformation()];

  // Tests.
  NSDate* dateStored = [[NSUserDefaults standardUserDefaults]
      objectForKey:metrics_mediator::kAppEnteredBackgroundDateKey];
  EXPECT_EQ(nil, dateStored);
  verifySwizzleHasBeenCalled();
  EXPECT_OCMOCK_VERIFY(startupInformation);
}

// Verifies that +logLaunchMetrics logs of the number of open tabs and nothing
// more if the background date is not set;
TEST_F(MetricsMediatorLogLaunchTest, logLaunchMetricsNoBackgroundDate) {
  // Setup.
  BOOL coldStart = NO;
  initiateMetricsMediator(coldStart, 32);

  id startupInformation =
      [OCMockObject mockForProtocol:@protocol(StartupInformation)];
  [[[startupInformation stub] andReturnValue:@(coldStart)] isColdStart];

  [[NSUserDefaults standardUserDefaults]
      removeObjectForKey:metrics_mediator::kAppEnteredBackgroundDateKey];

  // Action.
  [MetricsMediator
      logLaunchMetricsWithStartupInformation:startupInformation
                      browserViewInformation:getBrowserViewInformation()];

  // Tests.
  verifySwizzleHasBeenCalled();
}

// Tests that +logDateInUserDefaults logs the date in UserDefaults.
TEST(MetricsMediatorNoFixtureTest, logDateInUserDefaultsTest) {
  // Setup.
  [[NSUserDefaults standardUserDefaults]
      removeObjectForKey:metrics_mediator::kAppEnteredBackgroundDateKey];

  NSDate* lastAppClose = [[NSUserDefaults standardUserDefaults]
      objectForKey:metrics_mediator::kAppEnteredBackgroundDateKey];

  ASSERT_EQ(nil, lastAppClose);

  // Action.
  [MetricsMediator logDateInUserDefaults];

  // Setup.
  lastAppClose = [[NSUserDefaults standardUserDefaults]
      objectForKey:metrics_mediator::kAppEnteredBackgroundDateKey];
  EXPECT_NE(nil, lastAppClose);
}

#pragma mark - processCrashReportsPresentAtStartup tests.

class MetricsMediatorShutdownTypeTest : public testing::TestWithParam<int> {};

// Verifies that the Breakpad controller gets called appropriately when
// processCrashReportsPresentAtStartup is invoked.
//
// This parameterized test receives an int parameter that is the crash log
// count waiting to be processed.
TEST_P(MetricsMediatorShutdownTypeTest, ProcessCrashReportsPresentAtStartup) {
  // Create a MainController.
  MetricsMediator* metric_helper = [[MetricsMediator alloc] init];

  // Create a mock for BreakpadController and swizzle
  // +[BreakpadController sharedInstance] to return the mock instead of the
  // normal singleton instance.
  id mock_breakpad_controller =
      [OCMockObject mockForClass:[BreakpadController class]];

  id implementation_block = ^BreakpadController*(id self) {
    return mock_breakpad_controller;
  };
  ScopedBlockSwizzler breakpad_controller_shared_instance_swizzler(
      [BreakpadController class], @selector(sharedInstance),
      implementation_block);

  // Create the mock expectation for calling
  // -[BreakpadController getCrashReportCount].
  [mock_breakpad_controller cr_expectGetCrashReportCount:GetParam()];

  // Now call the method under test and verify that the Breakpad controller got
  // called appropriately.
  [metric_helper processCrashReportsPresentAtStartup];
  EXPECT_OCMOCK_VERIFY(mock_breakpad_controller);
}

INSTANTIATE_TEST_CASE_P(/* No InstantiationName */,
                        MetricsMediatorShutdownTypeTest,
                        ::testing::Values(0, 1, 42));

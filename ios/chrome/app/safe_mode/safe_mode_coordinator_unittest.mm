// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/app/safe_mode/safe_mode_coordinator.h"

#import <UIKit/UIKit.h>

#include "base/mac/scoped_nsobject.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(SafeModeCoordinatorTest, RootVC) {
  // Expect that starting a safe mode coordinator will populate the root view
  // controller.
  base::scoped_nsobject<UIWindow> window(
      [[UIWindow alloc] initWithFrame:CGRectMake(0, 0, 100, 100)]);
  EXPECT_TRUE([window rootViewController] == nil);
  base::scoped_nsobject<SafeModeCoordinator> safe_mode_coordinator(
      [[SafeModeCoordinator alloc] initWithWindow:window]);
  [safe_mode_coordinator start];
  EXPECT_FALSE([window rootViewController] == nil);
}

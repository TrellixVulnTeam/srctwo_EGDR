// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_METRICS_PREVIOUS_SESSION_INFO_PRIVATE_H_
#define IOS_CHROME_BROWSER_METRICS_PREVIOUS_SESSION_INFO_PRIVATE_H_

@interface PreviousSessionInfo (TestingOnly)

// Redefined to be read-write.
@property(nonatomic, assign) BOOL didSeeMemoryWarningShortlyBeforeTerminating;

// Redefined to be read-write.
@property(nonatomic, assign) BOOL isFirstSessionAfterUpgrade;

+ (void)resetSharedInstanceForTesting;

@end

#endif  // IOS_CHROME_BROWSER_METRICS_PREVIOUS_SESSION_INFO_PRIVATE_H_

// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

#import "remoting/ios/app/help_and_feedback.h"

#import "base/logging.h"

static HelpAndFeedback* g_helpAndFeedback;

@implementation HelpAndFeedback

#pragma mark - Public

- (void)presentFeedbackFlowWithContext:(NSString*)context {
  NSLog(@"Called presentFeedbackFlow");
}

#pragma mark - Static Properties

+ (void)setInstance:(HelpAndFeedback*)instance {
  DCHECK(!g_helpAndFeedback);
  g_helpAndFeedback = instance;
}

+ (HelpAndFeedback*)instance {
  DCHECK(g_helpAndFeedback);
  return g_helpAndFeedback;
}

@end

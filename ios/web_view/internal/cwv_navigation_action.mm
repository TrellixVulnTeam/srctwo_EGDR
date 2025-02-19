// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/web_view/public/cwv_navigation_action.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

@implementation CWVNavigationAction
@synthesize request = _request;
@synthesize userInitiated = _userInitiated;

- (instancetype)initWithRequest:(NSURLRequest*)request
                  userInitiated:(BOOL)userInitiated {
  self = [super init];
  if (self) {
    _userInitiated = userInitiated;
    _request = [request copy];
  }
  return self;
}

@end

// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <UIKit/UIKit.h>

#import "ios/web_view/shell/shell_app_delegate.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

int main(int argc, char* argv[]) {
  @autoreleasepool {
    return UIApplicationMain(argc, argv, nil,
                             NSStringFromClass([ShellAppDelegate class]));
  }
}

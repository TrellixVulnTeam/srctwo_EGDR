// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/util/top_view_controller.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace top_view_controller {

UIViewController* TopPresentedViewController() {
  return TopPresentedViewControllerFrom(
      [[UIApplication sharedApplication] keyWindow].rootViewController);
}
UIViewController* TopPresentedViewControllerFrom(
    UIViewController* base_view_controller) {
  UIViewController* topController = base_view_controller;
  for (UIViewController* controller = [topController presentedViewController];
       controller && ![controller isBeingDismissed];
       controller = [controller presentedViewController]) {
    topController = controller;
  }
  return topController;
}

}  // namespace top_view_controller

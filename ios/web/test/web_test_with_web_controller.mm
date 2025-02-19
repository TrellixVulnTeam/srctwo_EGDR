// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/web/test/web_test_with_web_controller.h"

#import "ios/web/web_state/web_state_impl.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace web {

WebTestWithWebController::WebTestWithWebController() {}

WebTestWithWebController::~WebTestWithWebController() {}

CRWWebController* WebTestWithWebController::web_controller() {
  return static_cast<web::WebStateImpl*>(web_state())->GetWebController();
}

}  // namespace web

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_WEB_TEST_WEB_TEST_WITH_WEB_CONTROLLER_H_
#define IOS_WEB_TEST_WEB_TEST_WITH_WEB_CONTROLLER_H_

#import "ios/web/public/test/web_test_with_web_state.h"
#import "ios/web/web_state/ui/crw_web_controller.h"

namespace web {

// Base test fixture to test CRWWebController.
class WebTestWithWebController : public WebTestWithWebState {
 protected:
  WebTestWithWebController();
  ~WebTestWithWebController() override;
  // Returns web controller for testing.
  CRWWebController* web_controller();
};

}  // namespace web

#endif  // IOS_WEB_TEST_WEB_TEST_WITH_WEB_CONTROLLER_H_

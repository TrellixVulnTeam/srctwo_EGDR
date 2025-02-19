// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/login/session/user_session_manager_test_api.h"

namespace chromeos {
namespace test {

UserSessionManagerTestApi::UserSessionManagerTestApi(
    UserSessionManager* session_manager)
    : session_manager_(session_manager) {
}

void UserSessionManagerTestApi::InjectStubUserContext(
    const UserContext& user_context) {
  session_manager_->InjectStubUserContext(user_context);
}

void UserSessionManagerTestApi::SetShouldLaunchBrowserInTests(
    bool should_launch_browser) {
  session_manager_->set_should_launch_browser_in_tests(should_launch_browser);
}

void UserSessionManagerTestApi::SetShouldObtainTokenHandleInTests(
    bool should_obtain_handle) {
  session_manager_->SetShouldObtainHandleInTests(should_obtain_handle);
}

}  // namespace test
}  // namespace chromeos

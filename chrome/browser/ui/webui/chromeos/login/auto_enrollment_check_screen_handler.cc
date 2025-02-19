// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/chromeos/login/auto_enrollment_check_screen_handler.h"

#include "chrome/browser/chromeos/login/oobe_screen.h"
#include "chrome/grit/generated_resources.h"
#include "components/login/localized_values_builder.h"

namespace {

const char kJsScreenPath[] = "login.AutoEnrollmentCheckScreen";

}  // namespace

namespace chromeos {

AutoEnrollmentCheckScreenHandler::AutoEnrollmentCheckScreenHandler()
    : BaseScreenHandler(kScreenId) {
  set_call_js_prefix(kJsScreenPath);
}

AutoEnrollmentCheckScreenHandler::~AutoEnrollmentCheckScreenHandler() {
  if (delegate_)
    delegate_->OnViewDestroyed(this);
}

void AutoEnrollmentCheckScreenHandler::Show() {
  if (!page_is_ready()) {
    show_on_init_ = true;
    return;
  }
  ShowScreen(kScreenId);
}

void AutoEnrollmentCheckScreenHandler::SetDelegate(Delegate* delegate) {
  delegate_ = delegate;
  if (page_is_ready())
    Initialize();
}

void AutoEnrollmentCheckScreenHandler::DeclareLocalizedValues(
    ::login::LocalizedValuesBuilder* builder) {
  builder->Add("autoEnrollmentCheckScreenHeader",
               IDS_AUTO_ENROLLMENT_CHECK_SCREEN_HEADER);
  builder->Add("autoEnrollmentCheckMessage",
               IDS_AUTO_ENROLLMENT_CHECK_SCREEN_MESSAGE);
}

void AutoEnrollmentCheckScreenHandler::Initialize() {
  if (!page_is_ready() || !delegate_)
    return;

  if (show_on_init_) {
    Show();
    show_on_init_ = false;
  }
}

void AutoEnrollmentCheckScreenHandler::RegisterMessages() {}

}  // namespace chromeos

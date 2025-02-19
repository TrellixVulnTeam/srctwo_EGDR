// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/startup/automation_infobar_delegate.h"

#include "chrome/browser/devtools/global_confirm_info_bar.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/grit/generated_resources.h"
#include "components/infobars/core/infobar.h"
#include "ui/base/l10n/l10n_util.h"

// static
void AutomationInfoBarDelegate::Create() {
  std::unique_ptr<ConfirmInfoBarDelegate> delegate(
      new AutomationInfoBarDelegate());
  GlobalConfirmInfoBar::Show(std::move(delegate));
}

AutomationInfoBarDelegate::AutomationInfoBarDelegate() {}

AutomationInfoBarDelegate::~AutomationInfoBarDelegate() {}

infobars::InfoBarDelegate::InfoBarIdentifier
AutomationInfoBarDelegate::GetIdentifier() const {
  return AUTOMATION_INFOBAR_DELEGATE;
}

bool AutomationInfoBarDelegate::ShouldExpire(
    const NavigationDetails& details) const {
  return false;
}

base::string16 AutomationInfoBarDelegate::GetMessageText() const {
  return l10n_util::GetStringUTF16(IDS_CONTROLLED_BY_AUTOMATION);
}

int AutomationInfoBarDelegate::GetButtons() const {
  return BUTTON_NONE;
}

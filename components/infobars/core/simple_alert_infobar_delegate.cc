// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/infobars/core/simple_alert_infobar_delegate.h"

#include <memory>

#include "components/infobars/core/infobar.h"
#include "components/infobars/core/infobar_manager.h"
#include "third_party/skia/include/core/SkBitmap.h"

// static
void SimpleAlertInfoBarDelegate::Create(
    infobars::InfoBarManager* infobar_manager,
    infobars::InfoBarDelegate::InfoBarIdentifier infobar_identifier,
    const gfx::VectorIcon* vector_icon,
    const base::string16& message,
    bool auto_expire) {
  infobar_manager->AddInfoBar(infobar_manager->CreateConfirmInfoBar(
      std::unique_ptr<ConfirmInfoBarDelegate>(new SimpleAlertInfoBarDelegate(
          infobar_identifier, vector_icon, message, auto_expire))));
}

SimpleAlertInfoBarDelegate::SimpleAlertInfoBarDelegate(
    infobars::InfoBarDelegate::InfoBarIdentifier infobar_identifier,
    const gfx::VectorIcon* vector_icon,
    const base::string16& message,
    bool auto_expire)
    : ConfirmInfoBarDelegate(),
      infobar_identifier_(infobar_identifier),
      vector_icon_(vector_icon),
      message_(message),
      auto_expire_(auto_expire) {}

SimpleAlertInfoBarDelegate::~SimpleAlertInfoBarDelegate() {
}

infobars::InfoBarDelegate::InfoBarIdentifier
SimpleAlertInfoBarDelegate::GetIdentifier() const {
  return infobar_identifier_;
}

const gfx::VectorIcon& SimpleAlertInfoBarDelegate::GetVectorIcon() const {
  return vector_icon_ ? *vector_icon_ : InfoBarDelegate::GetVectorIcon();
}

bool SimpleAlertInfoBarDelegate::ShouldExpire(
    const NavigationDetails& details) const {
  return auto_expire_ && ConfirmInfoBarDelegate::ShouldExpire(details);
}

base::string16 SimpleAlertInfoBarDelegate::GetMessageText() const {
  return message_;
}

int SimpleAlertInfoBarDelegate::GetButtons() const {
  return BUTTON_NONE;
}

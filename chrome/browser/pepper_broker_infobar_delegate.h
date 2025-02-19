// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PEPPER_BROKER_INFOBAR_DELEGATE_H_
#define CHROME_BROWSER_PEPPER_BROKER_INFOBAR_DELEGATE_H_

#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/macros.h"
#include "components/infobars/core/confirm_infobar_delegate.h"
#include "url/gurl.h"

class HostContentSettingsMap;
class InfoBarService;
class TabSpecificContentSettings;

namespace content {
class WebContents;
}

// Shows an infobar that asks the user whether a Pepper plugin is allowed
// to connect to its (privileged) broker. The user decision is made "sticky"
// by storing a content setting for the site.
class PepperBrokerInfoBarDelegate : public ConfirmInfoBarDelegate {
 public:
  // Determines whether the broker setting is allow, deny, or ask.  In the first
  // two cases, runs the callback directly.  In the third, creates a pepper
  // broker infobar and delegate and adds the infobar to the InfoBarService
  // associated with |web_contents|.
  static void Create(content::WebContents* web_contents,
                     const GURL& url,
                     const base::FilePath& plugin_path,
                     const base::Callback<void(bool)>& callback);

 private:
  PepperBrokerInfoBarDelegate(const GURL& url,
                              const base::FilePath& plugin_path,
                              HostContentSettingsMap* content_settings,
                              TabSpecificContentSettings* tab_content_settings,
                              const base::Callback<void(bool)>& callback);
  ~PepperBrokerInfoBarDelegate() override;

  // ConfirmInfoBarDelegate:
  infobars::InfoBarDelegate::InfoBarIdentifier GetIdentifier() const override;
  const gfx::VectorIcon& GetVectorIcon() const override;
  base::string16 GetMessageText() const override;
  base::string16 GetButtonLabel(InfoBarButton button) const override;
  bool Accept() override;
  bool Cancel() override;
  base::string16 GetLinkText() const override;
  GURL GetLinkURL() const override;

  void DispatchCallback(bool result);

  const GURL url_;
  const base::FilePath plugin_path_;
  HostContentSettingsMap* content_settings_;
  TabSpecificContentSettings* tab_content_settings_;
  base::Callback<void(bool)> callback_;

  DISALLOW_COPY_AND_ASSIGN(PepperBrokerInfoBarDelegate);
};

#endif  // CHROME_BROWSER_PEPPER_BROKER_INFOBAR_DELEGATE_H_

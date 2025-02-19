// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_SSL_IOS_SSL_BLOCKING_PAGE_H_
#define IOS_CHROME_BROWSER_SSL_IOS_SSL_BLOCKING_PAGE_H_

#include <string>
#include <vector>

#include "base/callback.h"
#include "base/macros.h"
#include "base/time/time.h"
#include "ios/chrome/browser/interstitials/ios_security_interstitial_page.h"
#include "net/ssl/ssl_info.h"

class IOSChromeControllerClient;
class GURL;

namespace security_interstitials {
class SSLErrorUI;
}

// This class is responsible for showing/hiding the interstitial page that is
// shown when a certificate error happens.
// It deletes itself when the interstitial page is closed.
class IOSSSLBlockingPage : public IOSSecurityInterstitialPage {
 public:
  ~IOSSSLBlockingPage() override;

  // Creates an SSL blocking page. If the blocking page isn't shown, the caller
  // is responsible for cleaning up the blocking page, otherwise the
  // interstitial takes ownership when shown. |options_mask| must be a bitwise
  // mask of SSLErrorUI::SSLErrorOptionsMask values.
  IOSSSLBlockingPage(web::WebState* web_state,
                     int cert_error,
                     const net::SSLInfo& ssl_info,
                     const GURL& request_url,
                     int options_mask,
                     const base::Time& time_triggered,
                     const base::Callback<void(bool)>& callback);

 protected:
  // InterstitialPageDelegate implementation.
  void CommandReceived(const std::string& command) override;
  void OnProceed() override;
  void OnDontProceed() override;
  void OverrideItem(web::NavigationItem* item) override;

  // SecurityInterstitialPage implementation:
  bool ShouldCreateNewNavigation() const override;
  void PopulateInterstitialStrings(
      base::DictionaryValue* load_time_data) const override;
  void AfterShow() override;

 private:
  void NotifyDenyCertificate();

  // Returns true if |options_mask| refers to a soft-overridable SSL error.
  static bool IsOverridable(int options_mask);

  base::Callback<void(bool)> callback_;
  const net::SSLInfo ssl_info_;
  const bool overridable_;  // The UI allows the user to override the error.

  // The user previously allowed a bad certificate, but the decision has now
  // expired.
  const bool expired_but_previously_allowed_;

  std::unique_ptr<IOSChromeControllerClient> controller_;
  std::unique_ptr<security_interstitials::SSLErrorUI> ssl_error_ui_;

  DISALLOW_COPY_AND_ASSIGN(IOSSSLBlockingPage);
};

#endif  // IOS_CHROME_BROWSER_SSL_IOS_SSL_BLOCKING_PAGE_H_

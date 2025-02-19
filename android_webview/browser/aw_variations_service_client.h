// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ANDROID_WEBVIEW_BROWSER_AW_VARIATIONS_SERVICE_CLIENT_H_
#define ANDROID_WEBVIEW_BROWSER_AW_VARIATIONS_SERVICE_CLIENT_H_

#include <string>

#include "base/macros.h"
#include "components/variations/service/variations_service_client.h"

namespace android_webview {

// AwVariationsServiceClient provides an implementation of
// VariationsServiceClient, all members are currently stubs for WebView.
class AwVariationsServiceClient : public variations::VariationsServiceClient {
 public:
  AwVariationsServiceClient();
  ~AwVariationsServiceClient() override;

 private:
  std::string GetApplicationLocale() override;
  base::Callback<base::Version(void)> GetVersionForSimulationCallback()
      override;
  net::URLRequestContextGetter* GetURLRequestContext() override;
  network_time::NetworkTimeTracker* GetNetworkTimeTracker() override;
  version_info::Channel GetChannel() override;
  bool OverridesRestrictParameter(std::string* parameter) override;

  DISALLOW_COPY_AND_ASSIGN(AwVariationsServiceClient);
};

}  // namespace android_webview

#endif  // ANDROID_WEBVIEW_BROWSER_AW_VARIATIONS_SERVICE_CLIENT_H_

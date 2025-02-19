// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_METRICS_VARIATIONS_CHROME_VARIATIONS_SERVICE_CLIENT_H_
#define CHROME_BROWSER_METRICS_VARIATIONS_CHROME_VARIATIONS_SERVICE_CLIENT_H_

#include <string>

#include "base/macros.h"
#include "components/variations/service/variations_service_client.h"

// ChromeVariationsServiceClient provides an implementation of
// VariationsServiceClient that depends on chrome/.
class ChromeVariationsServiceClient
    : public variations::VariationsServiceClient {
 public:
  ChromeVariationsServiceClient();
  ~ChromeVariationsServiceClient() override;

  // variations::VariationsServiceClient:
  std::string GetApplicationLocale() override;
  base::Callback<base::Version(void)> GetVersionForSimulationCallback()
      override;
  net::URLRequestContextGetter* GetURLRequestContext() override;
  network_time::NetworkTimeTracker* GetNetworkTimeTracker() override;
  version_info::Channel GetChannel() override;
  bool OverridesRestrictParameter(std::string* parameter) override;

  DISALLOW_COPY_AND_ASSIGN(ChromeVariationsServiceClient);
};

#endif  // CHROME_BROWSER_METRICS_VARIATIONS_CHROME_VARIATIONS_SERVICE_CLIENT_H_

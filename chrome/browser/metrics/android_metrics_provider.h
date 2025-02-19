// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_METRICS_ANDROID_METRICS_PROVIDER_H_
#define CHROME_BROWSER_METRICS_ANDROID_METRICS_PROVIDER_H_

#include "base/macros.h"
#include "components/metrics/metrics_provider.h"

namespace metrics {
class ChromeUserMetricsExtension;
}

// AndroidMetricsProvider provides Android-specific stability metrics.
class AndroidMetricsProvider : public metrics::MetricsProvider {
 public:
  AndroidMetricsProvider();
  ~AndroidMetricsProvider() override;

  // metrics::MetricsProvider:
  void ProvidePreviousSessionData(
      metrics::ChromeUserMetricsExtension* uma_proto) override;
  void ProvideCurrentSessionData(
      metrics::ChromeUserMetricsExtension* uma_proto) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(AndroidMetricsProvider);
};

#endif  // CHROME_BROWSER_METRICS_ANDROID_METRICS_PROVIDER_H_

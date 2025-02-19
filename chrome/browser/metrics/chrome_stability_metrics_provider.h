// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_METRICS_CHROME_STABILITY_METRICS_PROVIDER_H_
#define CHROME_BROWSER_METRICS_CHROME_STABILITY_METRICS_PROVIDER_H_

#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/process/kill.h"
#include "components/metrics/metrics_provider.h"
#include "components/metrics/stability_metrics_helper.h"
#include "content/public/browser/browser_child_process_observer.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"

class PrefService;

// ChromeStabilityMetricsProvider gathers and logs Chrome-specific stability-
// related metrics.
class ChromeStabilityMetricsProvider
    : public metrics::MetricsProvider,
      public content::BrowserChildProcessObserver,
      public content::NotificationObserver {
 public:
  explicit ChromeStabilityMetricsProvider(PrefService* local_state);
  ~ChromeStabilityMetricsProvider() override;

  // metrics::MetricsDataProvider:
  void OnRecordingEnabled() override;
  void OnRecordingDisabled() override;
  void ProvideStabilityMetrics(
      metrics::SystemProfileProto* system_profile_proto) override;
  void ClearSavedStabilityMetrics() override;

 private:
  FRIEND_TEST_ALL_PREFIXES(ChromeStabilityMetricsProviderTest,
                           BrowserChildProcessObserver);
  FRIEND_TEST_ALL_PREFIXES(ChromeStabilityMetricsProviderTest,
                           NotificationObserver);

  // content::NotificationObserver:
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) override;

  // content::BrowserChildProcessObserver:
  void BrowserChildProcessCrashed(
      const content::ChildProcessData& data,
      int exit_code) override;

  metrics::StabilityMetricsHelper helper_;

  // Registrar for receiving stability-related notifications.
  content::NotificationRegistrar registrar_;

  DISALLOW_COPY_AND_ASSIGN(ChromeStabilityMetricsProvider);
};

#endif  // CHROME_BROWSER_METRICS_CHROME_STABILITY_METRICS_PROVIDER_H_

// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_OMNIBOX_BROWSER_OMNIBOX_METRICS_PROVIDER_H_
#define COMPONENTS_OMNIBOX_BROWSER_OMNIBOX_METRICS_PROVIDER_H_

#include "base/macros.h"
#include "components/metrics/metrics_provider.h"
#include "components/metrics/proto/chrome_user_metrics_extension.pb.h"
#include "components/omnibox/browser/omnibox_event_global_tracker.h"

struct OmniboxLog;

// OmniboxMetricsProvider is responsible for filling out the |omnibox_event|
// section of the UMA proto.
class OmniboxMetricsProvider : public metrics::MetricsProvider {
 public:
  explicit OmniboxMetricsProvider(
      const base::Callback<bool(void)>& is_off_the_record_callback);
  ~OmniboxMetricsProvider() override;

  // metrics::MetricsDataProvider:
  void OnRecordingEnabled() override;
  void OnRecordingDisabled() override;
  void ProvideCurrentSessionData(
      metrics::ChromeUserMetricsExtension* uma_proto) override;

 private:
  // Called when a URL is opened from the Omnibox.
  void OnURLOpenedFromOmnibox(OmniboxLog* log);

  // Records the input text, available choices, and selected entry when the
  // user uses the Omnibox to open a URL.
  void RecordOmniboxOpenedURL(const OmniboxLog& log);

  // Subscription for receiving Omnibox event callbacks.
  std::unique_ptr<base::CallbackList<void(OmniboxLog*)>::Subscription>
      subscription_;

  // Saved cache of generated Omnibox event protos, to be copied into the UMA
  // proto when ProvideCurrentSessionData() is called.
  metrics::ChromeUserMetricsExtension omnibox_events_cache;

  // Callback passed in from the embedder that returns whether the user is
  // currently operating off-the-record.
  const base::Callback<bool(void)> is_off_the_record_callback_;

  DISALLOW_COPY_AND_ASSIGN(OmniboxMetricsProvider);
};

#endif  // COMPONENTS_OMNIBOX_BROWSER_OMNIBOX_METRICS_PROVIDER_H_

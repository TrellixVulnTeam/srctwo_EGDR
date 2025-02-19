// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PAGE_LOAD_METRICS_OBSERVERS_MULTI_TAB_LOADING_PAGE_LOAD_METRICS_OBSERVER_H_
#define CHROME_BROWSER_PAGE_LOAD_METRICS_OBSERVERS_MULTI_TAB_LOADING_PAGE_LOAD_METRICS_OBSERVER_H_

#include "base/macros.h"
#include "chrome/browser/page_load_metrics/page_load_metrics_observer.h"

namespace content {
class NavigationHandle;
}

namespace internal {

// Exposed for tests.
extern const char kHistogramMultiTabLoadingFirstContentfulPaint[];
extern const char kHistogramMultiTabLoadingForegroundToFirstContentfulPaint[];
extern const char kHistogramMultiTabLoadingFirstMeaningfulPaint[];
extern const char kHistogramMultiTabLoadingForegroundToFirstMeaningfulPaint[];
extern const char kHistogramMultiTabLoadingDomContentLoaded[];
extern const char kBackgroundHistogramMultiTabLoadingDomContentLoaded[];
extern const char kHistogramMultiTabLoadingLoad[];
extern const char kBackgroundHistogramMultiTabLoadingLoad[];

}  // namespace internal

// Observer responsible for recording core page load metrics while there are
// other loading tabs.
class MultiTabLoadingPageLoadMetricsObserver
    : public page_load_metrics::PageLoadMetricsObserver {
 public:
  MultiTabLoadingPageLoadMetricsObserver();
  ~MultiTabLoadingPageLoadMetricsObserver() override;

  // page_load_metrics::PageLoadMetricsObserver:
  page_load_metrics::PageLoadMetricsObserver::ObservePolicy OnStart(
      content::NavigationHandle* navigation_handle,
      const GURL& currently_committed_url,
      bool started_in_foreground) override;
  void OnFirstContentfulPaintInPage(
      const page_load_metrics::mojom::PageLoadTiming& timing,
      const page_load_metrics::PageLoadExtraInfo& extra_info) override;
  void OnFirstMeaningfulPaintInMainFrameDocument(
      const page_load_metrics::mojom::PageLoadTiming& timing,
      const page_load_metrics::PageLoadExtraInfo& info) override;
  void OnDomContentLoadedEventStart(
      const page_load_metrics::mojom::PageLoadTiming& timing,
      const page_load_metrics::PageLoadExtraInfo& info) override;
  void OnLoadEventStart(
      const page_load_metrics::mojom::PageLoadTiming& timing,
      const page_load_metrics::PageLoadExtraInfo& info) override;

 protected:
  // Overridden in testing.
  virtual bool IsAnyTabLoading(content::NavigationHandle* navigation_handle);

 private:
  DISALLOW_COPY_AND_ASSIGN(MultiTabLoadingPageLoadMetricsObserver);
};

#endif  // CHROME_BROWSER_PAGE_LOAD_METRICS_OBSERVERS_MULTI_TAB_LOADING_PAGE_LOAD_METRICS_OBSERVER_H_

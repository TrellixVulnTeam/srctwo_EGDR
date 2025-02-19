// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PAGE_LOAD_METRICS_METRICS_NAVIGATION_THROTTLE_H_
#define CHROME_BROWSER_PAGE_LOAD_METRICS_METRICS_NAVIGATION_THROTTLE_H_

#include <memory>

#include "base/macros.h"
#include "content/public/browser/navigation_throttle.h"

namespace page_load_metrics {

// This class is used to forward calls to the MetricsWebContentsObserver.
// Namely, WillStartRequest() is called on NavigationThrottles, but not on
// WebContentsObservers. Data from the NavigationHandle accessed at this point
// is used to obtain more reliable abort metrics (like page transition type).
class MetricsNavigationThrottle : public content::NavigationThrottle {
 public:
  static std::unique_ptr<content::NavigationThrottle> Create(
      content::NavigationHandle* handle);
  ~MetricsNavigationThrottle() override;

  // content::NavigationThrottle:
  content::NavigationThrottle::ThrottleCheckResult WillStartRequest() override;
  content::NavigationThrottle::ThrottleCheckResult WillProcessResponse()
      override;
  const char* GetNameForLogging() override;

 private:
  explicit MetricsNavigationThrottle(content::NavigationHandle* handle);

  DISALLOW_COPY_AND_ASSIGN(MetricsNavigationThrottle);
};

}  // namespace page_load_metrics

#endif  // CHROME_BROWSER_PAGE_LOAD_METRICS_METRICS_NAVIGATION_THROTTLE_H_

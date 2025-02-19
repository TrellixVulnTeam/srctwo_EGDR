// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SUBRESOURCE_FILTER_CONTENT_BROWSER_SUBRESOURCE_FILTER_OBSERVER_H_
#define COMPONENTS_SUBRESOURCE_FILTER_CONTENT_BROWSER_SUBRESOURCE_FILTER_OBSERVER_H_

#include "components/subresource_filter/core/common/activation_decision.h"
#include "components/subresource_filter/core/common/load_policy.h"

namespace content {
class NavigationHandle;
}  // namespace content

namespace subresource_filter {

struct ActivationState;

// Class to receive notifications of subresource filter events for a given
// WebContents. Registered with a SubresourceFilterObserverManager.
class SubresourceFilterObserver {
 public:
  virtual ~SubresourceFilterObserver() = default;

  // Called before the observer manager is destroyed. Observers must unregister
  // themselves by this point.
  virtual void OnSubresourceFilterGoingAway() {}

  // Called at most once per navigation when page activation is computed. This
  // will be called before ReadyToCommitNavigation.
  virtual void OnPageActivationComputed(
      content::NavigationHandle* navigation_handle,
      ActivationDecision activation_decision,
      const ActivationState& activation_state) {}

  // Called before navigation commit, either at the WillStartRequest stage or
  // WillRedirectRequest stage.
  virtual void OnSubframeNavigationEvaluated(
      content::NavigationHandle* navigation_handle,
      LoadPolicy load_policy) {}
};

}  // namespace subresource_filter

#endif  // COMPONENTS_SUBRESOURCE_FILTER_CONTENT_BROWSER_SUBRESOURCE_FILTER_OBSERVER_H_

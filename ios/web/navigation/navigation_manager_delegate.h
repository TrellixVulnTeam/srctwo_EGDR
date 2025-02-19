// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_WEB_NAVIGATION_NAVIGATION_MANAGER_DELEGATE_H_
#define IOS_WEB_NAVIGATION_NAVIGATION_MANAGER_DELEGATE_H_

#include <stddef.h>


@protocol CRWWebViewNavigationProxy;
class GURL;

namespace web {

struct LoadCommittedDetails;
class WebState;

// Delegate for NavigationManager to hand off parts of the navigation flow.
class NavigationManagerDelegate {
 public:
  virtual ~NavigationManagerDelegate() {}

  // Instructs the delegate to clear any transient content to prepare for new
  // navigation.
  virtual void ClearTransientContent() = 0;

  // Instructs the delegate to record page states (e.g. scroll position, form
  // values, whatever can be harvested) from the current page into the
  // navigation item.
  virtual void RecordPageStateInNavigationItem() = 0;

  // Instructs the delegate to update HTML5 History state of the page using the
  // current NavigationItem.
  virtual void UpdateHtml5HistoryState() = 0;

  // Instructs the delegate to perform book keeping in preparation for a new
  // navigation using a different user agent type.
  virtual void WillChangeUserAgentType() = 0;

  // Instructs the delegate to notify its delegates that the current navigation
  // item will be loaded.
  virtual void WillLoadCurrentItemWithUrl(const GURL&) = 0;

  // Instructs the delegate to load the current navigation item.
  virtual void LoadCurrentItem() = 0;

  // Instructs the delegate to load the current navigation item if the current
  // page has not loaded yet.
  virtual void LoadIfNecessary() = 0;

  // Instructs the delegate to reload.
  virtual void Reload() = 0;

  // Informs the delegate that committed navigation items have been pruned.
  virtual void OnNavigationItemsPruned(size_t pruned_item_count) = 0;

  // Informs the delegate that a navigation item has been changed.
  virtual void OnNavigationItemChanged() = 0;

  // Informs the delegate that a navigation item has been committed.
  virtual void OnNavigationItemCommitted(
      const LoadCommittedDetails& load_details) = 0;

  // Returns the WebState associated with this delegate.
  virtual WebState* GetWebState() = 0;

  // Returns a CRWWebViewNavigationProxy protocol that can be used to access
  // navigation related functions on the main WKWebView.
  virtual id<CRWWebViewNavigationProxy> GetWebViewNavigationProxy() const = 0;
};

}  // namespace web

#endif  // IOS_WEB_NAVIGATION_NAVIGATION_MANAGER_DELEGATE_H_

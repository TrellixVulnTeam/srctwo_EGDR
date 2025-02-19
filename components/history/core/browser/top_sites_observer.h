// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_HISTORY_CORE_BROWSER_TOP_SITES_OBSERVER_H_
#define COMPONENTS_HISTORY_CORE_BROWSER_TOP_SITES_OBSERVER_H_

#include "base/macros.h"

namespace history {

class TopSites;

// Interface for observing notifications from TopSites.
class TopSitesObserver {
 public:
  // An enum representing different for TopSitesChanged to happen.
  enum class ChangeReason {
    // TopSites was changed by most visited.
    MOST_VISITED,
    // TopSites was changed by add/remove/clear blacklist.
    BLACKLIST,
    // TopSites was changed by AddForcedURLs.
    FORCED_URL,
  };

  TopSitesObserver() {}
  virtual ~TopSitesObserver() {}

  // Is called when TopSites finishes loading.
  virtual void TopSitesLoaded(TopSites* top_sites) = 0;

  // Is called when either one of the most visited urls
  // changed, or one of the images changes.
  virtual void TopSitesChanged(TopSites* top_sites,
                               ChangeReason change_reason) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(TopSitesObserver);
};

}  // namespace history

#endif  // COMPONENTS_HISTORY_CORE_BROWSER_TOP_SITES_OBSERVER_H_

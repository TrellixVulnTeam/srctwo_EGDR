// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_HISTORY_IOS_BROWSER_WEB_STATE_TOP_SITES_OBSERVER_H_
#define COMPONENTS_HISTORY_IOS_BROWSER_WEB_STATE_TOP_SITES_OBSERVER_H_

#include "base/macros.h"
#include "ios/web/public/web_state/web_state_observer.h"
#include "ios/web/public/web_state/web_state_user_data.h"

namespace history {

class TopSites;

// WebStateTopSitesObserver forwards navigation events from web::WebState to
// TopSites.
class WebStateTopSitesObserver
    : public web::WebStateObserver,
      public web::WebStateUserData<WebStateTopSitesObserver> {
 public:
  ~WebStateTopSitesObserver() override;

  static void CreateForWebState(web::WebState* web_state, TopSites* top_sites);

 private:
  friend class web::WebStateUserData<WebStateTopSitesObserver>;

  WebStateTopSitesObserver(web::WebState* web_state, TopSites* top_sites);

  // web::WebStateObserver implementation.
  void NavigationItemCommitted(
      const web::LoadCommittedDetails& load_details) override;

  // Underlying TopSites instance, may be null during testing.
  TopSites* top_sites_;

  DISALLOW_COPY_AND_ASSIGN(WebStateTopSitesObserver);
};

}  // namespace history

#endif  // COMPONENTS_HISTORY_IOS_BROWSER_WEB_STATE_TOP_SITES_OBSERVER_H_

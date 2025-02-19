// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SYNC_SESSIONS_REVISIT_PAGE_REVISIT_BROADCASTER_H_
#define COMPONENTS_SYNC_SESSIONS_REVISIT_PAGE_REVISIT_BROADCASTER_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "components/sync_sessions/revisit/page_visit_observer.h"
#include "ui/base/page_transition_types.h"
#include "url/gurl.h"

namespace sync_sessions {
class SyncSessionsClient;
}

namespace sync_sessions {

class SessionsSyncManager;

// This class has the job of creating and holding onto the PageVisitObservers
// that are to be notified on page change for purposes of instrumenting
// revisists.
class PageRevisitBroadcaster {
 public:
  PageRevisitBroadcaster(SessionsSyncManager* manager,
                         SyncSessionsClient* sessions_client);
  ~PageRevisitBroadcaster();

  // Broadcasts to all observers the given page visit event. Should only be
  // called when the url changes.
  void OnPageVisit(const GURL& url, const ui::PageTransition transition);

 private:
  friend class SyncPageRevisitBroadcasterTest;

  // We convert between enums here for a couple reasons. We don't want to force
  // observers to depend on ui/, and the high bit masks don't work for emitting
  // histograms. Some of the high bit masks correspond to cases we're
  // particularly interested in and want to treat as first class values.
  static PageVisitObserver::TransitionType ConvertTransitionEnum(
      const ui::PageTransition original);

  // The client of this sync sessions datatype.
  SyncSessionsClient* const sessions_client_;

  std::vector<std::unique_ptr<PageVisitObserver>> revisit_observers_;

  DISALLOW_COPY_AND_ASSIGN(PageRevisitBroadcaster);
};

}  // namespace sync_sessions

#endif  // COMPONENTS_SYNC_SESSIONS_REVISIT_PAGE_REVISIT_BROADCASTER_H_

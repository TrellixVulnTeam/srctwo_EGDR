// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/feature_engagement/bookmark/bookmark_tracker.h"

#include "base/time/time.h"
#include "chrome/browser/feature_engagement/session_duration_updater.h"
#include "chrome/browser/feature_engagement/session_duration_updater_factory.h"
#include "chrome/browser/feature_engagement/tracker_factory.h"
#include "chrome/browser/metrics/desktop_session_duration/desktop_session_duration_tracker.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/pref_names.h"
#include "components/feature_engagement/public/event_constants.h"
#include "components/feature_engagement/public/feature_constants.h"
#include "components/feature_engagement/public/tracker.h"

namespace {

const int kDefaultPromoShowTimeInHours = 5;

}  // namespace

namespace feature_engagement {

BookmarkTracker::BookmarkTracker(
    Profile* profile,
    SessionDurationUpdater* session_duration_updater)
    : FeatureTracker(profile,
                     session_duration_updater,
                     &kIPHBookmarkFeature,
                     base::TimeDelta::FromHours(kDefaultPromoShowTimeInHours)) {
}

BookmarkTracker::BookmarkTracker(
    SessionDurationUpdater* session_duration_updater)
    : BookmarkTracker(nullptr, session_duration_updater) {}

BookmarkTracker::~BookmarkTracker() = default;

void BookmarkTracker::OnPromoClosed() {
  GetTracker()->Dismissed(kIPHBookmarkFeature);
}

void BookmarkTracker::OnBookmarkAdded() {
  GetTracker()->NotifyEvent(events::kBookmarkAdded);
}

void BookmarkTracker::OnVisitedKnownURL() {
  if (ShouldShowPromo())
    ShowPromo();
}

void BookmarkTracker::OnSessionTimeMet() {
  GetTracker()->NotifyEvent(events::kBookmarkSessionTimeMet);
}

void BookmarkTracker::ShowPromo() {
  // TODO: Call the promo.

  // Clears the flag for whether there is any in-product help being displayed.
  GetTracker()->Dismissed(kIPHBookmarkFeature);
}

}  // namespace feature_engagement

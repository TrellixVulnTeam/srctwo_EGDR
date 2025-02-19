// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_NTP_TILES_CONSTANTS_H_
#define COMPONENTS_NTP_TILES_CONSTANTS_H_

namespace base {
struct Feature;
}  // namespace base

namespace ntp_tiles {

// Name of the field trial to configure PopularSites.
extern const char kPopularSitesFieldTrialName[];

// This feature is enabled by default. Otherwise, users who need it would not
// get the right configuration timely enough. The configuration affects only
// Android or iOS users.
extern const base::Feature kPopularSitesBakedInContentFeature;

// Feature to allow the new Google favicon server for fetching favicons for Most
// Likely tiles on the New Tab Page.
extern const base::Feature kNtpMostLikelyFaviconsFromServerFeature;

// Feature to allow displaying lower resolution favicons for Tiles on the New
// Tab Page.
extern const base::Feature kLowerResolutionFaviconsFeature;

// Feature to provide site exploration tiles in addition to personal tiles.
extern const base::Feature kSiteExplorationUiFeature;

// Use this to find out whether the kNtpMostLikelyFaviconsFromServerFeature is
// enabled. This helper function abstracts iOS special way to override the
// feature (via command-line params).
// TODO(jkrcal): Remove once crbug.com/718926 is fixed.
bool AreNtpMostLikelyFaviconsFromServerEnabled();

}  // namespace ntp_tiles

#endif  // COMPONENTS_NTP_TILES_CONSTANTS_H_

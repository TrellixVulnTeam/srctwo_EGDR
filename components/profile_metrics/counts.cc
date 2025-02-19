// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/profile_metrics/counts.h"

#include "base/metrics/histogram_macros.h"

namespace profile_metrics {

void LogProfileMetricsCounts(const Counts& counts) {
  UMA_HISTOGRAM_COUNTS_100("Profile.NumberOfProfiles", counts.total);

  // Ignore other metrics if we have no profiles.
  if (counts.total > 0) {
    UMA_HISTOGRAM_COUNTS_100("Profile.NumberOfManagedProfiles",
                             counts.supervised);
    UMA_HISTOGRAM_COUNTS_100("Profile.PercentageOfManagedProfiles",
                             100 * counts.supervised / counts.total);
    UMA_HISTOGRAM_COUNTS_100("Profile.NumberOfSignedInProfiles",
                             counts.signedin);
    UMA_HISTOGRAM_COUNTS_100("Profile.NumberOfUnusedProfiles", counts.unused);
    UMA_HISTOGRAM_COUNTS_100("Profile.NumberOfSignedInProfilesWithGAIAIcons",
                             counts.gaia_icon);
    UMA_HISTOGRAM_COUNTS_100("Profile.NumberOfProfilesWithAuthErrors",
                             counts.auth_errors);
  }
}

}  // namespace profile_metrics

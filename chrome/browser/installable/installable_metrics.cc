// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/installable/installable_metrics.h"

#include "base/metrics/histogram_macros.h"

void InstallableMetrics::RecordMenuOpenHistogram(
    InstallabilityCheckStatus status) {
  UMA_HISTOGRAM_ENUMERATION("Webapp.InstallabilityCheckStatus.MenuOpen", status,
                            InstallabilityCheckStatus::COUNT);
}

void InstallableMetrics::RecordMenuItemAddToHomescreenHistogram(
    InstallabilityCheckStatus status) {
  UMA_HISTOGRAM_ENUMERATION(
      "Webapp.InstallabilityCheckStatus.MenuItemAddToHomescreen", status,
      InstallabilityCheckStatus::COUNT);
}

// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_METRICS_STABILITY_METRICS_HELPER_H_
#define COMPONENTS_METRICS_STABILITY_METRICS_HELPER_H_

#include "base/macros.h"
#include "base/process/kill.h"

class PrefRegistrySimple;
class PrefService;

namespace metrics {

class SystemProfileProto;

// StabilityMetricsHelper is a class that providers functionality common to
// different embedders' stability metrics providers.
class StabilityMetricsHelper {
 public:
  explicit StabilityMetricsHelper(PrefService* local_state);
  ~StabilityMetricsHelper();

  // Provides stability metrics.
  void ProvideStabilityMetrics(SystemProfileProto* system_profile_proto);

  // Clears the gathered stability metrics.
  void ClearSavedStabilityMetrics();

  // Records a browser child process crash.
  void BrowserChildProcessCrashed();

  // Logs the initiation of a page load.
  void LogLoadStarted();

  // Records a renderer process crash.
  void LogRendererCrash(bool was_extension_process,
                        base::TerminationStatus status,
                        int exit_code);

  // Records that a new renderer process was successfully launched.
  void LogRendererLaunched(bool was_extension_process);

  // Records a renderer process hang.
  void LogRendererHang();

  // Registers local state prefs used by this class.
  static void RegisterPrefs(PrefRegistrySimple* registry);

 private:
  // Increment an Integer pref value specified by |path|.
  void IncrementPrefValue(const char* path);

  // Increment a 64-bit Integer pref value specified by |path|.
  void IncrementLongPrefsValue(const char* path);

  PrefService* local_state_;

  DISALLOW_COPY_AND_ASSIGN(StabilityMetricsHelper);
};

}  // namespace metrics

#endif  // COMPONENTS_METRICS_STABILITY_METRICS_HELPER_H_

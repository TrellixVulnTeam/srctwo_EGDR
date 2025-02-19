// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_CRASH_APP_STATE_TRACKER_H_
#define CHROMECAST_CRASH_APP_STATE_TRACKER_H_

#include <string>

namespace chromecast {

class AppStateTracker {
 public:
  // Record |app_id| as the last app that attempted to launch.
  static void SetLastLaunchedApp(const std::string& app_id);

  // The current app becomes the previous app, |app_id| becomes the current app.
  static void SetCurrentApp(const std::string& app_id);

  // Returns the id of the app that was last attempted to launch.
  static std::string GetLastLaunchedApp();

  // Returns the id of the active app.
  static std::string GetCurrentApp();

  // Returns the id of the app which was previously active.
  static std::string GetPreviousApp();
};

}  // namespace chromecast

#endif  // CHROMECAST_CRASH_APP_STATE_TRACKER_H_

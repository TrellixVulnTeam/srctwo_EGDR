// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_INSTALLER_LAUNCHER_SUPPORT_CHROME_LAUNCHER_SUPPORT_H_
#define CHROME_INSTALLER_LAUNCHER_SUPPORT_CHROME_LAUNCHER_SUPPORT_H_

namespace base {
class FilePath;
}

namespace chrome_launcher_support {

enum InstallationLevel {
  USER_LEVEL_INSTALLATION,
  SYSTEM_LEVEL_INSTALLATION,
};

// Returns the path to an installed chrome.exe at the specified level, if it can
// be found in the registry. Prefers the installer from a multi-install, but may
// also return that of a single-install of Chrome if no multi-install exists.
// If |is_sxs| is true, gets the path to the SxS (Canary) version of chrome.exe.
base::FilePath GetChromePathForInstallationLevel(InstallationLevel level,
                                                 bool is_sxs);

// Returns the path to an installed chrome.exe, or an empty path. Prefers a
// system-level installation to a user-level installation. Uses the registry to
// identify a Chrome installation location. If |is_sxs| is true, gets the path
// to the SxS (Canary) version of chrome.exe. The file path returned (if any) is
// guaranteed to exist.
base::FilePath GetAnyChromePath(bool is_sxs);

}  // namespace chrome_launcher_support

#endif  // CHROME_INSTALLER_LAUNCHER_SUPPORT_CHROME_LAUNCHER_SUPPORT_H_

// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_SNAPSHOTS_SNAPSHOTS_UTIL_H_
#define IOS_CHROME_BROWSER_SNAPSHOTS_SNAPSHOTS_UTIL_H_

#include <vector>

#include "base/files/file_path.h"

// Clears the application snapshots taken by iOS.
void ClearIOSSnapshots();

// Adds to |snapshotsPaths| all the possible paths to the application's
// snapshots taken by iOS.
void GetSnapshotsPaths(std::vector<base::FilePath>* snapshotsPaths);

#endif  // IOS_CHROME_BROWSER_SNAPSHOTS_SNAPSHOTS_UTIL_H_

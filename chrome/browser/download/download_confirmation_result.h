// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_DOWNLOAD_DOWNLOAD_CONFIRMATION_RESULT_H_
#define CHROME_BROWSER_DOWNLOAD_DOWNLOAD_CONFIRMATION_RESULT_H_

// Result of RequestConfirmation delegate method for
// DownloadTargetDeterminerDelegate.
enum class DownloadConfirmationResult {
  // The user confirmed the path. Only use this value if the user was explicitly
  // shown the path or at least the filename being downloaded.
  CONFIRMED,

  // The operation failed due to a reason other than a user cancellation.
  FAILED,

  // The user cancelled.
  CANCELED,

  // User was not explicitly prompted, but continue with current path. The
  // delegate should use this value instead of CONFIRMED if the user was not
  // presented with some UI that explicitly called out the filename being
  // downloaded.
  CONTINUE_WITHOUT_CONFIRMATION
};

#endif  // CHROME_BROWSER_DOWNLOAD_DOWNLOAD_CONFIRMATION_RESULT_H_

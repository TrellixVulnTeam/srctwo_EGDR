// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_FEEDBACK_FEEDBACK_DIALOG_UTILS_H_
#define CHROME_BROWSER_FEEDBACK_FEEDBACK_DIALOG_UTILS_H_

class Browser;
class GURL;
class Profile;

// Utility functions for the feedback dialog.
namespace chrome {

// Get the GURL of the active tab when the feedback dialog was invoked, if
// any.
GURL GetTargetTabUrl(int session_id, int index);

// Get the profile that should be used to open the feedback dialog.
Profile* GetFeedbackProfile(Browser* browser);

}  // namespace chrome

#endif  // CHROME_BROWSER_FEEDBACK_FEEDBACK_DIALOG_UTILS_H_

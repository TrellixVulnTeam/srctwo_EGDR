// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_PROCESS_SINGLETON_DIALOG_LINUX_H_
#define CHROME_BROWSER_UI_PROCESS_SINGLETON_DIALOG_LINUX_H_

#include "base/strings/string16.h"

// Displays an error to the user when the ProcessSingleton cannot acquire the
// lock.  This runs the message loop itself as the browser message loop has not
// started by that point in the startup process.

// Shows the dialog, and returns once the dialog has been closed.
bool ShowProcessSingletonDialog(const base::string16& message,
                                const base::string16& relaunch_text);

#endif  // CHROME_BROWSER_UI_PROCESS_SINGLETON_DIALOG_LINUX_H_

// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROME_CHILD_PROCESS_WATCHER_H_
#define CHROME_BROWSER_CHROME_CHILD_PROCESS_WATCHER_H_

#include "base/macros.h"
#include "content/public/browser/browser_child_process_observer.h"

// ChromeChildProcessWatcher watches for crashed child processes.
class ChromeChildProcessWatcher : public content::BrowserChildProcessObserver {
 public:
  ChromeChildProcessWatcher();
  ~ChromeChildProcessWatcher() override;

 private:
  // content::BrowserChildProcessObserver:
  void BrowserChildProcessCrashed(const content::ChildProcessData& data,
                                  int exit_code) override;

  DISALLOW_COPY_AND_ASSIGN(ChromeChildProcessWatcher);
};

#endif  // CHROME_BROWSER_CHROME_CHILD_PROCESS_WATCHER_H_

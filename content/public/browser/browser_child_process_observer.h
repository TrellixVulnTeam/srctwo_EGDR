// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_BROWSER_CHILD_PROCESS_OBSERVER_H_
#define CONTENT_PUBLIC_BROWSER_BROWSER_CHILD_PROCESS_OBSERVER_H_

#include "content/common/content_export.h"

namespace content {

struct ChildProcessData;

// An observer API implemented by classes which are interested in browser child
// process events. Note that render processes cannot be observed through this
// interface; use RenderProcessHostObserver instead.
class CONTENT_EXPORT BrowserChildProcessObserver {
 public:
  // Called when a child process host has connected to a child process.
  // Note that |data.handle| may be invalid, if the child process connects to
  // the pipe before the process launcher's reply arrives.
  virtual void BrowserChildProcessHostConnected(const ChildProcessData& data) {}

  // Called when a child process has successfully launched and has connected to
  // it child process host. The |data.handle| is guaranteed to be valid.
  virtual void BrowserChildProcessLaunchedAndConnected(
      const ChildProcessData& data) {}

  // Called after a ChildProcessHost is disconnected from the child process.
  virtual void BrowserChildProcessHostDisconnected(
      const ChildProcessData& data) {}

  // Called when a child process disappears unexpectedly as a result of a crash.
  // |exit_code| contains the exit code from the process.
  virtual void BrowserChildProcessCrashed(const ChildProcessData& data,
                                          int exit_code) {}

  // Called when a child process disappears unexpectedly as a result of being
  // killed.
  // |exit_code| contains the exit code from the process.
  virtual void BrowserChildProcessKilled(const ChildProcessData& data,
                                         int exit_code) {}

 protected:
  // The observer can be destroyed on any thread.
  virtual ~BrowserChildProcessObserver() {}

  static void Add(BrowserChildProcessObserver* observer);
  static void Remove(BrowserChildProcessObserver* observer);
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_BROWSER_CHILD_PROCESS_OBSERVER_H_

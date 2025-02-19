// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/base/user_input_monitor.h"

#include "base/logging.h"

namespace media {

#ifdef DISABLE_USER_INPUT_MONITOR
std::unique_ptr<UserInputMonitor> UserInputMonitor::Create(
    const scoped_refptr<base::SingleThreadTaskRunner>& io_task_runner,
    const scoped_refptr<base::SingleThreadTaskRunner>& ui_task_runner) {
  return nullptr;
}
#endif  // DISABLE_USER_INPUT_MONITOR

UserInputMonitor::UserInputMonitor() : key_press_counter_references_(0) {}

UserInputMonitor::~UserInputMonitor() {
  DCHECK_EQ(0u, key_press_counter_references_);
}

void UserInputMonitor::EnableKeyPressMonitoring() {
  base::AutoLock auto_lock(lock_);
  ++key_press_counter_references_;
  if (key_press_counter_references_ == 1) {
    StartKeyboardMonitoring();
    DVLOG(2) << "Started keyboard monitoring.";
  }
}

void UserInputMonitor::DisableKeyPressMonitoring() {
  base::AutoLock auto_lock(lock_);
  DCHECK_NE(key_press_counter_references_, 0u);
  --key_press_counter_references_;
  if (key_press_counter_references_ == 0) {
    StopKeyboardMonitoring();
    DVLOG(2) << "Stopped keyboard monitoring.";
  }
}

}  // namespace media

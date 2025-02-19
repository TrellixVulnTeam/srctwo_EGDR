// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/public/cpp/system/handle_signal_tracker.h"

#include "base/synchronization/lock.h"
#include "mojo/public/cpp/system/handle_signals_state.h"

namespace mojo {

HandleSignalTracker::HandleSignalTracker(Handle handle,
                                         MojoHandleSignals signals)
    : high_watcher_(FROM_HERE, SimpleWatcher::ArmingPolicy::MANUAL),
      low_watcher_(FROM_HERE, SimpleWatcher::ArmingPolicy::MANUAL) {
  MojoResult rv = high_watcher_.Watch(
      handle, signals, MOJO_WATCH_CONDITION_SATISFIED,
      base::Bind(&HandleSignalTracker::OnNotify, base::Unretained(this)));
  DCHECK_EQ(MOJO_RESULT_OK, rv);

  rv = low_watcher_.Watch(
      handle, signals, MOJO_WATCH_CONDITION_NOT_SATISFIED,
      base::Bind(&HandleSignalTracker::OnNotify, base::Unretained(this)));
  DCHECK_EQ(MOJO_RESULT_OK, rv);

  last_known_state_ = handle.QuerySignalsState();

  Arm();
}

HandleSignalTracker::~HandleSignalTracker() = default;

void HandleSignalTracker::Arm() {
  // Arm either the low watcher or high watcher. We cycle until one of them
  // succeeds, which should almost always happen within two iterations.
  bool arm_low_watcher = true;
  for (;;) {
    MojoResult ready_result;
    SimpleWatcher& watcher = arm_low_watcher ? low_watcher_ : high_watcher_;
    MojoResult result = watcher.Arm(&ready_result, &last_known_state_);
    if (result == MOJO_RESULT_OK) {
      // Successfully armed one of the watchers, so we can go back to waiting
      // for a notification.
      return;
    }

    DCHECK_EQ(MOJO_RESULT_FAILED_PRECONDITION, result);
    if (ready_result == MOJO_RESULT_FAILED_PRECONDITION && !arm_low_watcher) {
      // The high watcher failed to arm because the watched signal will never
      // be satisfied again. We can also return in this case, and
      // |last_known_state_| will remain with its current value indefinitely.
      return;
    }
    arm_low_watcher = !arm_low_watcher;
  }
}

void HandleSignalTracker::OnNotify(MojoResult result,
                                   const HandleSignalsState& state) {
  last_known_state_ = state;
  Arm();
  if (notification_callback_)
    notification_callback_.Run(state);
}

}  // namespace mojo

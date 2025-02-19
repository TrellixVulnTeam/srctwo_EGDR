// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_TIMERS_ALARM_TIMER_CHROMEOS_H_
#define COMPONENTS_TIMERS_ALARM_TIMER_CHROMEOS_H_

#include <memory>

#include "base/files/file_descriptor_watcher_posix.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "base/time/time.h"
#include "base/timer/timer.h"

namespace base {
struct PendingTask;
}

namespace timers {
// The class implements a timer that is capable of waking the system up from a
// suspended state. For example, this is useful for running tasks that are
// needed for maintaining network connectivity, like sending heartbeat messages.
// Currently, this feature is only available on Chrome OS systems running linux
// version 3.11 or higher. On all other platforms, the AlarmTimer behaves
// exactly the same way as a regular Timer.
//
// An AlarmTimer instance can only be used from the sequence on which it was
// instantiated. Start() and Stop() must be called from a thread that supports
// FileDescriptorWatcher.
class AlarmTimer : public base::Timer {
 public:
  ~AlarmTimer() override;

  // Timer overrides.
  void Stop() override;
  void Reset() override;

 protected:
  AlarmTimer(bool retain_user_task, bool is_repeating);

 private:
  // Called when |alarm_fd_| is readable without blocking. Reads data from
  // |alarm_fd_| and calls OnTimerFired().
  void OnAlarmFdReadableWithoutBlocking();

  // Called when the timer fires. Runs the callback.
  void OnTimerFired();

  // Tracks whether the timer has the ability to wake the system up from
  // suspend. This is a runtime check because we won't know if the system
  // supports being woken up from suspend until the constructor actually tries
  // to set it up.
  bool CanWakeFromSuspend() const;

  // Timer file descriptor.
  const int alarm_fd_;

  // Watches |alarm_fd_|.
  std::unique_ptr<base::FileDescriptorWatcher::Controller> alarm_fd_watcher_;

  // Posts tasks to the sequence on which this AlarmTimer was instantiated.
  const scoped_refptr<base::SequencedTaskRunner> origin_task_runner_ =
      base::SequencedTaskRunnerHandle::Get();

  // Keeps track of the user task we want to run. A new one is constructed every
  // time Reset() is called.
  std::unique_ptr<base::PendingTask> pending_task_;

  // Used to invalidate pending callbacks.
  base::WeakPtrFactory<AlarmTimer> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(AlarmTimer);
};

// As its name suggests, a OneShotAlarmTimer runs a given task once.  It does
// not remember the task that was given to it after it has fired and does not
// repeat.  Useful for fire-and-forget tasks.
class OneShotAlarmTimer : public AlarmTimer {
 public:
  OneShotAlarmTimer();
  ~OneShotAlarmTimer() override;
};

// A RepeatingAlarmTimer takes a task and delay and repeatedly runs the task
// using the specified delay as an interval between the runs until it is
// explicitly stopped.  It remembers both the task and the delay it was given
// after it fires.
class RepeatingAlarmTimer : public AlarmTimer {
 public:
  RepeatingAlarmTimer();
  ~RepeatingAlarmTimer() override;
};

// A SimpleAlarmTimer only fires once but remembers the task that it was given
// even after it has fired.  Useful if you want to run the same task multiple
// times but not at a regular interval.
class SimpleAlarmTimer : public AlarmTimer {
 public:
  SimpleAlarmTimer();
  ~SimpleAlarmTimer() override;
};

}  // namespace timers

#endif  // COMPONENTS_TIMERS_ALARM_TIMER_CHROMEOS_H_

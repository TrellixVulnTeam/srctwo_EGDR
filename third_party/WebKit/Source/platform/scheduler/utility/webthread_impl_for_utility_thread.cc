// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/scheduler/utility/webthread_impl_for_utility_thread.h"

#include "base/threading/thread_task_runner_handle.h"

namespace blink {
namespace scheduler {

WebThreadImplForUtilityThread::WebThreadImplForUtilityThread()
    : task_runner_(base::ThreadTaskRunnerHandle::Get()),
      thread_id_(base::PlatformThread::CurrentId()) {}

WebThreadImplForUtilityThread::~WebThreadImplForUtilityThread() {}

blink::WebScheduler* WebThreadImplForUtilityThread::Scheduler() const {
  NOTIMPLEMENTED();
  return nullptr;
}

blink::PlatformThreadId WebThreadImplForUtilityThread::ThreadId() const {
  return thread_id_;
}

base::SingleThreadTaskRunner* WebThreadImplForUtilityThread::GetTaskRunner()
    const {
  return task_runner_.get();
}

scheduler::SingleThreadIdleTaskRunner*
WebThreadImplForUtilityThread::GetIdleTaskRunner() const {
  NOTIMPLEMENTED();
  return nullptr;
}

void WebThreadImplForUtilityThread::Init() {}

}  // namespace scheduler
}  // namespace blink

// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/test/frame_load_waiter.h"

#include "base/location.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"

namespace content {

FrameLoadWaiter::FrameLoadWaiter(RenderFrame* frame)
    : RenderFrameObserver(frame) {
}

void FrameLoadWaiter::Wait() {
  if (did_load_)
    return;

  run_loop_.Run();
}

void FrameLoadWaiter::DidFinishLoad() {
  did_load_ = true;
  // Post a task to quit instead of quitting directly, since the load completion
  // may trigger other IPCs that tests are expecting.
  base::ThreadTaskRunnerHandle::Get()->PostTask(FROM_HERE,
                                                run_loop_.QuitClosure());
}

void FrameLoadWaiter::OnDestruct() {
  delete this;
}

}  // namespace content

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/test/thread_test_helper.h"

#include <utility>

#include "base/bind.h"
#include "base/location.h"
#include "base/threading/thread_restrictions.h"

namespace base {

ThreadTestHelper::ThreadTestHelper(
    scoped_refptr<SequencedTaskRunner> target_sequence)
    : test_result_(false),
      target_sequence_(std::move(target_sequence)),
      done_event_(WaitableEvent::ResetPolicy::AUTOMATIC,
                  WaitableEvent::InitialState::NOT_SIGNALED) {}

bool ThreadTestHelper::Run() {
  if (!target_sequence_->PostTask(
          FROM_HERE, base::BindOnce(&ThreadTestHelper::RunOnSequence, this))) {
    return false;
  }
  base::ThreadRestrictions::ScopedAllowWait allow_wait;
  done_event_.Wait();
  return test_result_;
}

void ThreadTestHelper::RunTest() { set_test_result(true); }

ThreadTestHelper::~ThreadTestHelper() {}

void ThreadTestHelper::RunOnSequence() {
  RunTest();
  done_event_.Signal();
}

}  // namespace base

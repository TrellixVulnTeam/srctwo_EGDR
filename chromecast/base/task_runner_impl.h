// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_BASE_TASK_RUNNER_IMPL_H_
#define CHROMECAST_BASE_TASK_RUNNER_IMPL_H_

#include <stdint.h>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "chromecast/public/task_runner.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace chromecast {

// Implementation of public TaskRunner interface that just calls
// base::ThreadTaskRunnerHandle at construction time and uses it to post.
class TaskRunnerImpl : public TaskRunner {
 public:
  TaskRunnerImpl();
  ~TaskRunnerImpl() override;

  bool PostTask(Task* task, uint64_t delay_milliseconds) override;

  const scoped_refptr<base::SingleThreadTaskRunner>& runner() const {
    return runner_;
  }

 private:
  const scoped_refptr<base::SingleThreadTaskRunner> runner_;

  DISALLOW_COPY_AND_ASSIGN(TaskRunnerImpl);
};

}  // namespace chromecast

#endif  // CHROMECAST_BASE_TASK_RUNNER_IMPL_H_

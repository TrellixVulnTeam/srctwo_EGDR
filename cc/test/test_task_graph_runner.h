// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_TEST_TEST_TASK_GRAPH_RUNNER_H_
#define CC_TEST_TEST_TASK_GRAPH_RUNNER_H_

#include "base/macros.h"
#include "base/threading/simple_thread.h"
#include "cc/raster/single_thread_task_graph_runner.h"

namespace cc {

class TestTaskGraphRunner : public SingleThreadTaskGraphRunner {
 public:
  TestTaskGraphRunner();
  ~TestTaskGraphRunner() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(TestTaskGraphRunner);
};

}  // namespace cc

#endif  // CC_TEST_TEST_TASK_GRAPH_RUNNER_H_

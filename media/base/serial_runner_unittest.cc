// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <memory>

#include "base/bind.h"
#include "base/debug/stack_trace.h"
#include "base/macros.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/single_thread_task_runner.h"
#include "media/base/pipeline_status.h"
#include "media/base/serial_runner.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace media {

class SerialRunnerTest : public ::testing::Test {
 public:
  SerialRunnerTest()
      : inside_start_(false), done_called_(false), done_status_(PIPELINE_OK) {}
  ~SerialRunnerTest() override {}

  void RunSerialRunner() {
    message_loop_.task_runner()->PostTask(
        FROM_HERE, base::Bind(&SerialRunnerTest::StartRunnerInternal,
                              base::Unretained(this), bound_fns_));
    base::RunLoop().RunUntilIdle();
  }

  // Pushes a bound function to the queue that will run its callback with
  // |status|. called(i) returns whether the i'th bound function pushed to the
  // queue was called while running the SerialRunner.
  void PushBoundFunction(PipelineStatus status) {
    bound_fns_.Push(base::Bind(&SerialRunnerTest::RunBoundFunction,
                               base::Unretained(this),
                               status,
                               called_.size()));
    called_.push_back(false);
  }

  void PushBoundClosure() {
    bound_fns_.Push(base::Bind(&SerialRunnerTest::RunBoundClosure,
                               base::Unretained(this),
                               called_.size()));
    called_.push_back(false);
  }

  void PushClosure() {
    bound_fns_.Push(base::Bind(&SerialRunnerTest::RunClosure,
                               base::Unretained(this),
                               called_.size()));
    called_.push_back(false);
  }

  // Push a bound function to the queue that will delete the SerialRunner,
  // which should cancel all remaining queued work.
  void PushCancellation() {
    bound_fns_.Push(base::Bind(&SerialRunnerTest::CancelSerialRunner,
                               base::Unretained(this)));
  }

  // Queries final status of pushed functions and done callback. Valid only
  // after calling RunSerialRunner().
  bool called(size_t index) { return called_[index]; }
  bool done_called() { return done_called_; }
  PipelineStatus done_status() { return done_status_; }

 private:
  void RunBoundFunction(PipelineStatus status,
                        size_t index,
                        const PipelineStatusCB& status_cb) {
    EXPECT_EQ(index == 0u, inside_start_)
        << "First bound function should run on same stack as "
        << "SerialRunner::Run() while all others should not\n"
        << base::debug::StackTrace().ToString();

    called_[index] = true;
    status_cb.Run(status);
  }

  void RunBoundClosure(size_t index,
                       const base::Closure& done_cb) {
    EXPECT_EQ(index == 0u, inside_start_)
        << "First bound function should run on same stack as "
        << "SerialRunner::Run() while all others should not\n"
        << base::debug::StackTrace().ToString();

    called_[index] = true;
    done_cb.Run();
  }

  void RunClosure(size_t index) {
    EXPECT_EQ(index == 0u, inside_start_)
        << "First bound function should run on same stack as "
        << "SerialRunner::Run() while all others should not\n"
        << base::debug::StackTrace().ToString();

    called_[index] = true;
  }

  void StartRunnerInternal(const SerialRunner::Queue& bound_fns) {
    inside_start_ = true;
    runner_ = SerialRunner::Run(bound_fns_, base::Bind(
        &SerialRunnerTest::DoneCallback, base::Unretained(this)));
    inside_start_ = false;
  }

  void DoneCallback(PipelineStatus status) {
    EXPECT_FALSE(inside_start_)
        << "Done callback should not run on same stack as SerialRunner::Run()\n"
        << base::debug::StackTrace().ToString();

    done_called_ = true;
    done_status_ = status;
  }

  void CancelSerialRunner(const PipelineStatusCB& status_cb) {
    // Tasks run by |runner_| shouldn't reset it, hence we post a task to do so.
    message_loop_.task_runner()->PostTask(
        FROM_HERE, base::Bind(&SerialRunnerTest::ResetSerialRunner,
                              base::Unretained(this)));
    status_cb.Run(PIPELINE_OK);
  }

  void ResetSerialRunner() {
    runner_.reset();
  }

  base::MessageLoop message_loop_;
  SerialRunner::Queue bound_fns_;
  std::unique_ptr<SerialRunner> runner_;

  // Used to enforce calling stack guarantees of the API.
  bool inside_start_;

  // Tracks whether the i'th bound function was called.
  std::vector<bool> called_;

  // Tracks whether the final done callback was called + resulting status.
  bool done_called_;
  PipelineStatus done_status_;

  DISALLOW_COPY_AND_ASSIGN(SerialRunnerTest);
};

TEST_F(SerialRunnerTest, Empty) {
  RunSerialRunner();

  EXPECT_TRUE(done_called());
  EXPECT_EQ(PIPELINE_OK, done_status());
}

TEST_F(SerialRunnerTest, Single) {
  PushBoundFunction(PIPELINE_OK);
  RunSerialRunner();

  EXPECT_TRUE(called(0));
  EXPECT_TRUE(done_called());
  EXPECT_EQ(PIPELINE_OK, done_status());
}

TEST_F(SerialRunnerTest, Single_Error) {
  PushBoundFunction(PIPELINE_ERROR_ABORT);
  RunSerialRunner();

  EXPECT_TRUE(called(0));
  EXPECT_TRUE(done_called());
  EXPECT_EQ(PIPELINE_ERROR_ABORT, done_status());
}

TEST_F(SerialRunnerTest, Single_Cancel) {
  PushBoundFunction(PIPELINE_OK);
  PushCancellation();
  RunSerialRunner();

  EXPECT_TRUE(called(0));
  EXPECT_FALSE(done_called());
}

TEST_F(SerialRunnerTest, Multiple) {
  PushBoundFunction(PIPELINE_OK);
  PushBoundFunction(PIPELINE_OK);
  RunSerialRunner();

  EXPECT_TRUE(called(0));
  EXPECT_TRUE(called(1));
  EXPECT_TRUE(done_called());
  EXPECT_EQ(PIPELINE_OK, done_status());
}

TEST_F(SerialRunnerTest, Multiple_Error) {
  PushBoundFunction(PIPELINE_ERROR_ABORT);
  PushBoundFunction(PIPELINE_OK);
  RunSerialRunner();

  EXPECT_TRUE(called(0));
  EXPECT_FALSE(called(1));  // A bad status cancels remaining work.
  EXPECT_TRUE(done_called());
  EXPECT_EQ(PIPELINE_ERROR_ABORT, done_status());
}

TEST_F(SerialRunnerTest, Multiple_Cancel) {
  PushBoundFunction(PIPELINE_OK);
  PushCancellation();
  PushBoundFunction(PIPELINE_OK);
  RunSerialRunner();

  EXPECT_TRUE(called(0));
  EXPECT_FALSE(called(1));
  EXPECT_FALSE(done_called());
}

TEST_F(SerialRunnerTest, BoundClosure) {
  PushBoundClosure();
  RunSerialRunner();

  EXPECT_TRUE(called(0));
  EXPECT_TRUE(done_called());
  EXPECT_EQ(PIPELINE_OK, done_status());
}

TEST_F(SerialRunnerTest, Closure) {
  PushClosure();
  RunSerialRunner();

  EXPECT_TRUE(called(0));
  EXPECT_TRUE(done_called());
  EXPECT_EQ(PIPELINE_OK, done_status());
}

}  // namespace media

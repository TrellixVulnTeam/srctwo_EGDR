// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/public/cpp/system/simple_watcher.h"

#include <memory>

#include "base/bind.h"
#include "base/callback.h"
#include "base/macros.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/threading/thread_task_runner_handle.h"
#include "mojo/public/c/system/types.h"
#include "mojo/public/cpp/system/message_pipe.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace mojo {
namespace {

template <typename Handler>
void RunResultHandler(Handler f, MojoResult result) {
  f(result);
}

template <typename Handler>
SimpleWatcher::ReadyCallback OnReady(Handler f) {
  return base::Bind(&RunResultHandler<Handler>, f);
}

SimpleWatcher::ReadyCallback NotReached() {
  return OnReady([](MojoResult) { NOTREACHED(); });
}

class SimpleWatcherTest : public testing::Test {
 public:
  SimpleWatcherTest() {}
  ~SimpleWatcherTest() override {}

 private:
  base::MessageLoop message_loop_;

  DISALLOW_COPY_AND_ASSIGN(SimpleWatcherTest);
};

TEST_F(SimpleWatcherTest, WatchBasic) {
  ScopedMessagePipeHandle a, b;
  CreateMessagePipe(nullptr, &a, &b);

  bool notified = false;
  base::RunLoop run_loop;
  SimpleWatcher b_watcher(FROM_HERE, SimpleWatcher::ArmingPolicy::AUTOMATIC);
  EXPECT_EQ(MOJO_RESULT_OK,
            b_watcher.Watch(b.get(), MOJO_HANDLE_SIGNAL_READABLE,
                            OnReady([&](MojoResult result) {
                              EXPECT_EQ(MOJO_RESULT_OK, result);
                              notified = true;
                              run_loop.Quit();
                            })));
  EXPECT_TRUE(b_watcher.IsWatching());

  EXPECT_EQ(MOJO_RESULT_OK, WriteMessageRaw(a.get(), "hello", 5, nullptr, 0,
                                            MOJO_WRITE_MESSAGE_FLAG_NONE));
  run_loop.Run();
  EXPECT_TRUE(notified);

  b_watcher.Cancel();
}

TEST_F(SimpleWatcherTest, WatchUnsatisfiable) {
  ScopedMessagePipeHandle a, b;
  CreateMessagePipe(nullptr, &a, &b);
  a.reset();

  SimpleWatcher b_watcher(FROM_HERE, SimpleWatcher::ArmingPolicy::MANUAL);
  EXPECT_EQ(
      MOJO_RESULT_OK,
      b_watcher.Watch(b.get(), MOJO_HANDLE_SIGNAL_READABLE, NotReached()));
  EXPECT_EQ(MOJO_RESULT_FAILED_PRECONDITION, b_watcher.Arm());
}

TEST_F(SimpleWatcherTest, WatchInvalidHandle) {
  ScopedMessagePipeHandle a, b;
  CreateMessagePipe(nullptr, &a, &b);
  a.reset();
  b.reset();

  SimpleWatcher b_watcher(FROM_HERE, SimpleWatcher::ArmingPolicy::AUTOMATIC);
  EXPECT_EQ(
      MOJO_RESULT_INVALID_ARGUMENT,
      b_watcher.Watch(b.get(), MOJO_HANDLE_SIGNAL_READABLE, NotReached()));
  EXPECT_FALSE(b_watcher.IsWatching());
}

TEST_F(SimpleWatcherTest, Cancel) {
  ScopedMessagePipeHandle a, b;
  CreateMessagePipe(nullptr, &a, &b);

  base::RunLoop run_loop;
  SimpleWatcher b_watcher(FROM_HERE, SimpleWatcher::ArmingPolicy::AUTOMATIC);
  EXPECT_EQ(
      MOJO_RESULT_OK,
      b_watcher.Watch(b.get(), MOJO_HANDLE_SIGNAL_READABLE, NotReached()));
  EXPECT_TRUE(b_watcher.IsWatching());
  b_watcher.Cancel();
  EXPECT_FALSE(b_watcher.IsWatching());

  // This should never trigger the watcher.
  EXPECT_EQ(MOJO_RESULT_OK, WriteMessageRaw(a.get(), "hello", 5, nullptr, 0,
                                            MOJO_WRITE_MESSAGE_FLAG_NONE));

  base::ThreadTaskRunnerHandle::Get()->PostTask(FROM_HERE,
                                                run_loop.QuitClosure());
  run_loop.Run();
}

TEST_F(SimpleWatcherTest, CancelOnClose) {
  ScopedMessagePipeHandle a, b;
  CreateMessagePipe(nullptr, &a, &b);

  base::RunLoop run_loop;
  SimpleWatcher b_watcher(FROM_HERE, SimpleWatcher::ArmingPolicy::AUTOMATIC);
  EXPECT_EQ(MOJO_RESULT_OK,
            b_watcher.Watch(b.get(), MOJO_HANDLE_SIGNAL_READABLE,
                            OnReady([&](MojoResult result) {
                              EXPECT_EQ(MOJO_RESULT_CANCELLED, result);
                              run_loop.Quit();
                            })));
  EXPECT_TRUE(b_watcher.IsWatching());

  // This should trigger the watcher above.
  b.reset();

  run_loop.Run();

  EXPECT_FALSE(b_watcher.IsWatching());
}

TEST_F(SimpleWatcherTest, CancelOnDestruction) {
  ScopedMessagePipeHandle a, b;
  CreateMessagePipe(nullptr, &a, &b);
  base::RunLoop run_loop;
  {
    SimpleWatcher b_watcher(FROM_HERE, SimpleWatcher::ArmingPolicy::AUTOMATIC);
    EXPECT_EQ(
        MOJO_RESULT_OK,
        b_watcher.Watch(b.get(), MOJO_HANDLE_SIGNAL_READABLE, NotReached()));
    EXPECT_TRUE(b_watcher.IsWatching());

    // |b_watcher| should be cancelled when it goes out of scope.
  }

  // This should never trigger the watcher above.
  EXPECT_EQ(MOJO_RESULT_OK, WriteMessageRaw(a.get(), "hello", 5, nullptr, 0,
                                            MOJO_WRITE_MESSAGE_FLAG_NONE));
  base::ThreadTaskRunnerHandle::Get()->PostTask(FROM_HERE,
                                                run_loop.QuitClosure());
  run_loop.Run();
}

TEST_F(SimpleWatcherTest, CloseAndCancel) {
  ScopedMessagePipeHandle a, b;
  CreateMessagePipe(nullptr, &a, &b);

  SimpleWatcher b_watcher(FROM_HERE, SimpleWatcher::ArmingPolicy::AUTOMATIC);
  EXPECT_EQ(MOJO_RESULT_OK,
            b_watcher.Watch(b.get(), MOJO_HANDLE_SIGNAL_READABLE,
                            OnReady([](MojoResult result) { FAIL(); })));
  EXPECT_TRUE(b_watcher.IsWatching());

  // This should trigger the watcher above...
  b.reset();
  // ...but the watcher is cancelled first.
  b_watcher.Cancel();

  EXPECT_FALSE(b_watcher.IsWatching());

  base::RunLoop().RunUntilIdle();
}

TEST_F(SimpleWatcherTest, UnarmedCancel) {
  ScopedMessagePipeHandle a, b;
  CreateMessagePipe(nullptr, &a, &b);

  SimpleWatcher b_watcher(FROM_HERE, SimpleWatcher::ArmingPolicy::MANUAL);
  base::RunLoop loop;
  EXPECT_EQ(MOJO_RESULT_OK,
            b_watcher.Watch(b.get(), MOJO_HANDLE_SIGNAL_READABLE,
                            base::Bind(
                                [](base::RunLoop* loop, MojoResult result) {
                                  EXPECT_EQ(result, MOJO_RESULT_CANCELLED);
                                  loop->Quit();
                                },
                                &loop)));

  // This message write will not wake up the watcher since the watcher isn't
  // armed. Instead, the cancellation will dispatch due to the reset below.
  EXPECT_EQ(MOJO_RESULT_OK, WriteMessageRaw(a.get(), "hello", 5, nullptr, 0,
                                            MOJO_WRITE_MESSAGE_FLAG_NONE));
  b.reset();
  loop.Run();
}

TEST_F(SimpleWatcherTest, ManualArming) {
  ScopedMessagePipeHandle a, b;
  CreateMessagePipe(nullptr, &a, &b);

  SimpleWatcher b_watcher(FROM_HERE, SimpleWatcher::ArmingPolicy::MANUAL);
  base::RunLoop loop;
  EXPECT_EQ(MOJO_RESULT_OK,
            b_watcher.Watch(b.get(), MOJO_HANDLE_SIGNAL_READABLE,
                            base::Bind(
                                [](base::RunLoop* loop, MojoResult result) {
                                  EXPECT_EQ(result, MOJO_RESULT_OK);
                                  loop->Quit();
                                },
                                &loop)));
  EXPECT_EQ(MOJO_RESULT_OK, b_watcher.Arm());

  EXPECT_EQ(MOJO_RESULT_OK, WriteMessageRaw(a.get(), "hello", 5, nullptr, 0,
                                            MOJO_WRITE_MESSAGE_FLAG_NONE));
  loop.Run();
}

TEST_F(SimpleWatcherTest, ManualArmOrNotifyWhileSignaled) {
  ScopedMessagePipeHandle a, b;
  CreateMessagePipe(nullptr, &a, &b);

  base::RunLoop loop1;
  SimpleWatcher b_watcher1(FROM_HERE, SimpleWatcher::ArmingPolicy::MANUAL);
  bool notified1 = false;
  EXPECT_EQ(MOJO_RESULT_OK,
            b_watcher1.Watch(
                b.get(), MOJO_HANDLE_SIGNAL_READABLE,
                base::Bind(
                    [](base::RunLoop* loop, bool* notified, MojoResult result) {
                      EXPECT_EQ(result, MOJO_RESULT_OK);
                      *notified = true;
                      loop->Quit();
                    },
                    &loop1, &notified1)));

  base::RunLoop loop2;
  SimpleWatcher b_watcher2(FROM_HERE, SimpleWatcher::ArmingPolicy::MANUAL);
  bool notified2 = false;
  EXPECT_EQ(MOJO_RESULT_OK,
            b_watcher2.Watch(
                b.get(), MOJO_HANDLE_SIGNAL_READABLE,
                base::Bind(
                    [](base::RunLoop* loop, bool* notified, MojoResult result) {
                      EXPECT_EQ(result, MOJO_RESULT_OK);
                      *notified = true;
                      loop->Quit();
                    },
                    &loop2, &notified2)));

  // First ensure that |b| is readable.
  EXPECT_EQ(MOJO_RESULT_OK, b_watcher1.Arm());
  EXPECT_EQ(MOJO_RESULT_OK, WriteMessageRaw(a.get(), "hello", 5, nullptr, 0,
                                            MOJO_WRITE_MESSAGE_FLAG_NONE));
  loop1.Run();

  EXPECT_TRUE(notified1);
  EXPECT_FALSE(notified2);
  notified1 = false;

  // Now verify that ArmOrNotify results in a notification.
  b_watcher2.ArmOrNotify();
  loop2.Run();

  EXPECT_FALSE(notified1);
  EXPECT_TRUE(notified2);
}

}  // namespace
}  // namespace mojo

// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/protocol/fake_message_pipe.h"

#include "base/bind.h"
#include "base/location.h"
#include "base/memory/ptr_util.h"
#include "base/threading/thread_task_runner_handle.h"
#include "remoting/base/compound_buffer.h"
#include "remoting/protocol/fake_message_pipe_wrapper.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/protobuf/src/google/protobuf/message_lite.h"

namespace remoting {
namespace protocol {

FakeMessagePipe::FakeMessagePipe(bool asynchronous)
    : asynchronous_(asynchronous) {}

FakeMessagePipe::~FakeMessagePipe() = default;

std::unique_ptr<FakeMessagePipeWrapper> FakeMessagePipe::Wrap() {
  return base::MakeUnique<FakeMessagePipeWrapper>(this);
}

void FakeMessagePipe::Start(EventHandler* event_handler) {
  ASSERT_TRUE(event_handler_ == nullptr);
  ASSERT_TRUE(event_handler != nullptr);
  event_handler_ = event_handler;
}

void FakeMessagePipe::Send(google::protobuf::MessageLite* message,
                           const base::Closure& done) {
  if (asynchronous_) {
    base::ThreadTaskRunnerHandle::Get()->PostTask(FROM_HERE,
        base::Bind([](FakeMessagePipe* me,
                      google::protobuf::MessageLite* message,
                      const base::Closure& done) {
                     me->SendImpl(message, done);
                   },
                   base::Unretained(this),
                   base::Unretained(message),
                   done));
    return;
  }
  SendImpl(message, done);
}

void FakeMessagePipe::Receive(std::unique_ptr<CompoundBuffer> message) {
  if (asynchronous_) {
    base::ThreadTaskRunnerHandle::Get()->PostTask(FROM_HERE,
        base::Bind([](FakeMessagePipe* me,
                      std::unique_ptr<CompoundBuffer> message) {
                     me->ReceiveImpl(std::move(message));
                   },
                   base::Unretained(this),
                   base::Passed(std::move(message))));
    return;
  }

  ReceiveImpl(std::move(message));
}

void FakeMessagePipe::OpenPipe() {
  if (asynchronous_) {
    base::ThreadTaskRunnerHandle::Get()->PostTask(FROM_HERE, base::Bind(
        [](FakeMessagePipe* me) {
          me->OpenPipeImpl();
        },
        base::Unretained(this)));
    return;
  }

  OpenPipeImpl();
}

void FakeMessagePipe::ClosePipe() {
  if (asynchronous_) {
    base::ThreadTaskRunnerHandle::Get()->PostTask(FROM_HERE, base::Bind(
        [](FakeMessagePipe* me) {
          me->ClosePipeImpl();
        },
        base::Unretained(this)));
    return;
  }

  ClosePipeImpl();
}

void FakeMessagePipe::SendImpl(
    google::protobuf::MessageLite* message,
    const base::Closure& done) {
  ASSERT_TRUE(pipe_opened_);

  std::string message_string;
  message->SerializeToString(&message_string);
  sent_messages_.push(message_string);

  if (done) {
    done.Run();
  }
}

void FakeMessagePipe::ReceiveImpl(std::unique_ptr<CompoundBuffer> message) {
  ASSERT_TRUE(pipe_opened_);
  ASSERT_TRUE(event_handler_ != nullptr);
  if (message) {
    message->Lock();
  }
  event_handler_->OnMessageReceived(std::move(message));
}

void FakeMessagePipe::OpenPipeImpl() {
  ASSERT_FALSE(pipe_opened_);
  ASSERT_TRUE(event_handler_ != nullptr);
  pipe_opened_ = true;
  event_handler_->OnMessagePipeOpen();
}

void FakeMessagePipe::ClosePipeImpl() {
  ASSERT_TRUE(pipe_opened_);
  ASSERT_TRUE(event_handler_ != nullptr);
  pipe_opened_ = false;
  event_handler_->OnMessagePipeClosed();
}

}  // namespace protocol
}  // namespace remoting

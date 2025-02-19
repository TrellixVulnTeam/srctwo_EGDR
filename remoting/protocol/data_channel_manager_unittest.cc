// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/protocol/data_channel_manager.h"

#include <map>
#include <utility>

#include "base/bind.h"
#include "base/memory/ptr_util.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "remoting/base/compound_buffer.h"
#include "remoting/proto/test.pb.h"
#include "remoting/protocol/fake_message_pipe.h"
#include "remoting/protocol/fake_message_pipe_wrapper.h"
#include "remoting/protocol/named_message_pipe_handler.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace remoting {
namespace protocol {

namespace {

class FakeNamedMessagePipeHandler final : public NamedMessagePipeHandler {
 public:
  FakeNamedMessagePipeHandler(const std::string& name,
                         std::unique_ptr<MessagePipe> pipe,
                         const std::string& expected_data)
      : NamedMessagePipeHandler(name, std::move(pipe)),
        expected_data_(expected_data) {
    handlers_[name] = this;
  }

  int received_message_count() const { return received_message_count_; }

  static FakeNamedMessagePipeHandler* Find(const std::string& name);

  static int instance_count() { return handlers_.size(); }

  // NamedMessagePipeHandler implementation.
  void OnIncomingMessage(std::unique_ptr<CompoundBuffer> message) override;

  // Public functions for test cases.
  void Close() { NamedMessagePipeHandler::Close(); }

  bool connected() const { return NamedMessagePipeHandler::connected(); }

  void Send(const google::protobuf::MessageLite& message,
            const base::Closure& done);

 protected:
  ~FakeNamedMessagePipeHandler() override {
    EXPECT_EQ(handlers_.erase(pipe_name()), 1U);
  }

 private:
  static std::map<std::string, FakeNamedMessagePipeHandler*> handlers_;

  const std::string expected_data_;
  int received_message_count_ = 0;
};

void FakeNamedMessagePipeHandler::OnIncomingMessage(
    std::unique_ptr<CompoundBuffer> message) {
  ASSERT_TRUE(message != nullptr);
  std::string content;
  content.resize(expected_data_.size());
  message->CopyTo(&(content[0]), content.size());
  ASSERT_EQ(content, expected_data_);
  received_message_count_++;
}

// static
FakeNamedMessagePipeHandler* FakeNamedMessagePipeHandler::Find(
    const std::string& name) {
  auto it = handlers_.find(name);
  if (it == handlers_.end()) {
    return nullptr;
  }
  return it->second;
}

void FakeNamedMessagePipeHandler::Send(
    const google::protobuf::MessageLite& message,
    const base::Closure& done) {
  if (connected()) {
    NamedMessagePipeHandler::Send(message, done);
    return;
  }
#if DCHECK_IS_ON()
  ASSERT_DEATH_IF_SUPPORTED(
    NamedMessagePipeHandler::Send(message, done),
    "connected");
#endif
}

// static
std::map<std::string, FakeNamedMessagePipeHandler*>
FakeNamedMessagePipeHandler::handlers_;

void TestDataChannelManagerFullMatch(bool asynchronous) {
  base::MessageLoop message_loop;
  DataChannelManager manager;
  manager.RegisterCreateHandlerCallback("FullMatch", base::Bind(
      [](const std::string& expected_data,
         const std::string& name,
         std::unique_ptr<MessagePipe> pipe) -> void {
        new FakeNamedMessagePipeHandler(name, std::move(pipe), expected_data);
      },
      "FullMatchContent"));
  manager.RegisterCreateHandlerCallback("AnotherFullMatch", base::Bind(
      [](const std::string& expected_data,
         const std::string& name,
         std::unique_ptr<MessagePipe> pipe) -> void {
        new FakeNamedMessagePipeHandler(name, std::move(pipe), expected_data);
      },
      "AnotherFullMatchContent"));

  FakeMessagePipe pipe1(asynchronous);
  FakeMessagePipe pipe2(asynchronous);
  ASSERT_TRUE(manager.OnIncomingDataChannel("FullMatch", pipe1.Wrap()));
  ASSERT_TRUE(manager.OnIncomingDataChannel("AnotherFullMatch", pipe2.Wrap()));
  base::RunLoop().RunUntilIdle();

  FakeNamedMessagePipeHandler* handler1 =
      FakeNamedMessagePipeHandler::Find("FullMatch");
  FakeNamedMessagePipeHandler* handler2 =
      FakeNamedMessagePipeHandler::Find("AnotherFullMatch");
  ASSERT_TRUE(handler1 != nullptr);
  ASSERT_TRUE(handler2 != nullptr);

  pipe1.OpenPipe();
  base::RunLoop().RunUntilIdle();
  {
    DataChannelManagerTestProto message;
    int sent = 0;
    base::Closure sent_callback = base::Bind([](int* sent) {
          (*sent)++;
        },
        base::Unretained(&sent));
    ASSERT_TRUE(handler1->connected());
    handler1->Send(message, sent_callback);
    ASSERT_FALSE(handler2->connected());
    handler2->Send(message, sent_callback);

    base::RunLoop().RunUntilIdle();
    ASSERT_EQ(sent, 1);
  }

  pipe2.OpenPipe();
  base::RunLoop().RunUntilIdle();
  {
    DataChannelManagerTestProto message;
    int sent = 0;
    base::Closure sent_callback = base::Bind([](int* sent) {
          (*sent)++;
        },
        base::Unretained(&sent));
    ASSERT_TRUE(handler2->connected());

    handler1->Send(message, sent_callback);
    handler2->Send(message, sent_callback);

    base::RunLoop().RunUntilIdle();
    ASSERT_EQ(sent, 2);
  }

  {
    std::string content;
    auto message = base::MakeUnique<CompoundBuffer>();
    content = "FullMatchContent";
    message->AppendCopyOf(&(content[0]), content.size());
    pipe1.Receive(std::move(message));

    message = base::MakeUnique<CompoundBuffer>();
    content = "AnotherFullMatchContent";
    message->AppendCopyOf(&(content[0]), content.size());
    pipe2.Receive(std::move(message));

    base::RunLoop().RunUntilIdle();
    ASSERT_EQ(handler1->received_message_count(), 1);
    ASSERT_EQ(handler2->received_message_count(), 1);
  }

  pipe2.ClosePipe();
  if (asynchronous) {
    base::RunLoop().RunUntilIdle();
  }
  ASSERT_TRUE(FakeNamedMessagePipeHandler::Find("AnotherFullMatch") == nullptr);

  handler1->Close();
  ASSERT_TRUE(FakeNamedMessagePipeHandler::Find("FullMatch") == nullptr);

  ASSERT_EQ(FakeNamedMessagePipeHandler::instance_count(), 0);
}

void TestDataChannelManagerMultipleRegistrations(bool asynchronous) {
  base::MessageLoop message_loop;
  DataChannelManager manager;
  manager.RegisterCreateHandlerCallback("FullMatch", base::Bind(
      [](const std::string& expected_data,
         const std::string& name,
         std::unique_ptr<MessagePipe> pipe) -> void {
        new FakeNamedMessagePipeHandler(name, std::move(pipe), expected_data);
      },
      "FullMatchContent"));
  manager.RegisterCreateHandlerCallback("Prefix-", base::Bind(
      [](const std::string& expected_data,
         const std::string& name,
         std::unique_ptr<MessagePipe> pipe) -> void {
        new FakeNamedMessagePipeHandler(name, std::move(pipe), expected_data);
      },
      "PrefixMatchContent"));

  FakeMessagePipe pipe1(asynchronous);
  FakeMessagePipe pipe2(asynchronous);
  FakeMessagePipe pipe3(asynchronous);
  FakeMessagePipe pipe4(asynchronous);
  ASSERT_TRUE(manager.OnIncomingDataChannel("Prefix-1", pipe1.Wrap()));
  ASSERT_TRUE(manager.OnIncomingDataChannel("Prefix-2", pipe2.Wrap()));
  ASSERT_FALSE(manager.OnIncomingDataChannel(
      "PrefixShouldNotMatch", pipe3.Wrap()));
  ASSERT_TRUE(manager.OnIncomingDataChannel("FullMatch", pipe4.Wrap()));
  base::RunLoop().RunUntilIdle();

  FakeNamedMessagePipeHandler* handler1 =
      FakeNamedMessagePipeHandler::Find("Prefix-1");
  FakeNamedMessagePipeHandler* handler2 =
      FakeNamedMessagePipeHandler::Find("Prefix-2");
  FakeNamedMessagePipeHandler* handler3 =
      FakeNamedMessagePipeHandler::Find("PrefixShouldNotMatch");
  FakeNamedMessagePipeHandler* handler4 =
      FakeNamedMessagePipeHandler::Find("FullMatch");
  ASSERT_TRUE(handler1 != nullptr);
  ASSERT_TRUE(handler2 != nullptr);
  ASSERT_TRUE(handler3 == nullptr);
  ASSERT_TRUE(handler4 != nullptr);

  handler1->Close();
  handler2->Close();
  handler4->Close();
  base::RunLoop().RunUntilIdle();
}

}  // namespace

TEST(DataChannelManagerTest, FullMatchWithSynchronousPipe) {
  TestDataChannelManagerFullMatch(false);
}

TEST(DataChannelManagerTest, FullMatchWithAsynchronousPipe) {
  TestDataChannelManagerFullMatch(true);
}

TEST(DataChannelManagerTest, MultipleRegistrationsWithSynchronousPipe) {
  TestDataChannelManagerMultipleRegistrations(false);
}

TEST(DataChannelManagerTest, MultipleRegistrationsWithAsynchronousPipe) {
  TestDataChannelManagerMultipleRegistrations(true);
}

}  // namespace protocol
}  // namespace remoting

// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/protocol/ice_transport.h"

#include <utility>

#include "base/bind.h"
#include "base/location.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "jingle/glue/thread_wrapper.h"
#include "net/url_request/url_request_context_getter.h"
#include "remoting/base/url_request.h"
#include "remoting/protocol/chromium_port_allocator_factory.h"
#include "remoting/protocol/connection_tester.h"
#include "remoting/protocol/fake_authenticator.h"
#include "remoting/protocol/message_channel_factory.h"
#include "remoting/protocol/message_pipe.h"
#include "remoting/protocol/transport_context.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/libjingle_xmpp/xmllite/xmlelement.h"

using testing::_;

namespace remoting {
namespace protocol {

namespace {

// Send 100 messages 1024 bytes each. UDP messages are sent with 10ms delay
// between messages (about 1 second for 100 messages).
const int kMessageSize = 1024;
const int kMessages = 100;
const char kChannelName[] = "test_channel";

ACTION_P2(QuitRunLoopOnCounter, run_loop, counter) {
  --(*counter);
  EXPECT_GE(*counter, 0);
  if (*counter == 0)
    run_loop->Quit();
}

class MockChannelCreatedCallback {
 public:
  MOCK_METHOD1(OnDone, void(MessagePipe* socket));
};

class TestTransportEventHandler : public IceTransport::EventHandler {
 public:
  typedef base::Callback<void(ErrorCode error)> ErrorCallback;

  TestTransportEventHandler() {}
  ~TestTransportEventHandler() {}

  void set_error_callback(const ErrorCallback& callback) {
    error_callback_ = callback;
  }

  // IceTransport::EventHandler interface.
  void OnIceTransportRouteChange(const std::string& channel_name,
                              const TransportRoute& route) override {}
  void OnIceTransportError(ErrorCode error) override {
    error_callback_.Run(error);
  }

 private:
  ErrorCallback error_callback_;

  DISALLOW_COPY_AND_ASSIGN(TestTransportEventHandler);
};

}  // namespace

class IceTransportTest : public testing::Test {
 public:
  IceTransportTest() {
    jingle_glue::JingleThreadWrapper::EnsureForCurrentMessageLoop();
    network_settings_ =
        NetworkSettings(NetworkSettings::NAT_TRAVERSAL_OUTGOING);
  }

  void TearDown() override {
    client_message_pipe_.reset();
    host_message_pipe_.reset();
    client_transport_.reset();
    host_transport_.reset();
    base::RunLoop().RunUntilIdle();
  }

  void ProcessTransportInfo(std::unique_ptr<IceTransport>* target_transport,
                            std::unique_ptr<buzz::XmlElement> transport_info) {
    base::ThreadTaskRunnerHandle::Get()->PostDelayedTask(
        FROM_HERE, base::Bind(&IceTransportTest::DeliverTransportInfo,
                              base::Unretained(this), target_transport,
                              base::Passed(&transport_info)),
        transport_info_delay_);
  }

  void DeliverTransportInfo(std::unique_ptr<IceTransport>* target_transport,
                            std::unique_ptr<buzz::XmlElement> transport_info) {
    ASSERT_TRUE(target_transport);
    EXPECT_TRUE(
        (*target_transport)->ProcessTransportInfo(transport_info.get()));
  }

  void InitializeConnection() {
    jingle_glue::JingleThreadWrapper::EnsureForCurrentMessageLoop();

    host_transport_.reset(new IceTransport(
        new TransportContext(nullptr,
                             base::MakeUnique<ChromiumPortAllocatorFactory>(),
                             nullptr, network_settings_, TransportRole::SERVER),
        &host_event_handler_));
    if (!host_authenticator_) {
      host_authenticator_.reset(
          new FakeAuthenticator(FakeAuthenticator::ACCEPT));
    }

    client_transport_.reset(new IceTransport(
        new TransportContext(nullptr,
                             base::MakeUnique<ChromiumPortAllocatorFactory>(),
                             nullptr, network_settings_, TransportRole::CLIENT),
        &client_event_handler_));
    if (!client_authenticator_) {
      client_authenticator_.reset(
          new FakeAuthenticator(FakeAuthenticator::ACCEPT));
    }

    host_event_handler_.set_error_callback(base::Bind(
        &IceTransportTest::OnTransportError, base::Unretained(this)));
    client_event_handler_.set_error_callback(base::Bind(
        &IceTransportTest::OnTransportError, base::Unretained(this)));

    // Start both transports.
    host_transport_->Start(
        host_authenticator_.get(),
        base::Bind(&IceTransportTest::ProcessTransportInfo,
                   base::Unretained(this), &client_transport_));
    client_transport_->Start(
        client_authenticator_.get(),
        base::Bind(&IceTransportTest::ProcessTransportInfo,
                   base::Unretained(this), &host_transport_));
  }

  void WaitUntilConnected() {
    run_loop_.reset(new base::RunLoop());

    int counter = 2;
    EXPECT_CALL(client_channel_callback_, OnDone(_))
        .WillOnce(QuitRunLoopOnCounter(run_loop_.get(), &counter));
    EXPECT_CALL(host_channel_callback_, OnDone(_))
        .WillOnce(QuitRunLoopOnCounter(run_loop_.get(), &counter));

    run_loop_->Run();

    EXPECT_TRUE(client_message_pipe_.get());
    EXPECT_TRUE(host_message_pipe_.get());
  }

  void OnClientChannelCreated(std::unique_ptr<MessagePipe> message_pipe) {
    client_message_pipe_ = std::move(message_pipe);
    client_channel_callback_.OnDone(client_message_pipe_.get());
  }

  void OnHostChannelCreated(std::unique_ptr<MessagePipe> message_pipe) {
    host_message_pipe_ = std::move(message_pipe);
    host_channel_callback_.OnDone(host_message_pipe_.get());
  }

  void OnTransportError(ErrorCode error) {
    LOG(ERROR) << "Transport Error";
    error_ = error;
    run_loop_->Quit();
  }

 protected:
  base::MessageLoopForIO message_loop_;
  std::unique_ptr<base::RunLoop> run_loop_;

  NetworkSettings network_settings_;

  base::TimeDelta transport_info_delay_;

  std::unique_ptr<IceTransport> host_transport_;
  TestTransportEventHandler host_event_handler_;
  std::unique_ptr<FakeAuthenticator> host_authenticator_;

  std::unique_ptr<IceTransport> client_transport_;
  TestTransportEventHandler client_event_handler_;
  std::unique_ptr<FakeAuthenticator> client_authenticator_;

  MockChannelCreatedCallback client_channel_callback_;
  MockChannelCreatedCallback host_channel_callback_;

  std::unique_ptr<MessagePipe> client_message_pipe_;
  std::unique_ptr<MessagePipe> host_message_pipe_;

  ErrorCode error_ = OK;
};

TEST_F(IceTransportTest, DataStream) {
  InitializeConnection();

  client_transport_->GetChannelFactory()->CreateChannel(
      kChannelName, base::Bind(&IceTransportTest::OnClientChannelCreated,
                               base::Unretained(this)));
  host_transport_->GetChannelFactory()->CreateChannel(
      kChannelName, base::Bind(&IceTransportTest::OnHostChannelCreated,
                               base::Unretained(this)));

  WaitUntilConnected();

  MessagePipeConnectionTester tester(host_message_pipe_.get(),
                                     client_message_pipe_.get(), kMessageSize,
                                     kMessages);
  tester.RunAndCheckResults();
}

TEST_F(IceTransportTest, MuxDataStream) {
  InitializeConnection();

  client_transport_->GetMultiplexedChannelFactory()->CreateChannel(
      kChannelName, base::Bind(&IceTransportTest::OnClientChannelCreated,
                               base::Unretained(this)));
  host_transport_->GetMultiplexedChannelFactory()->CreateChannel(
      kChannelName, base::Bind(&IceTransportTest::OnHostChannelCreated,
                               base::Unretained(this)));

  WaitUntilConnected();

  MessagePipeConnectionTester tester(host_message_pipe_.get(),
                                     client_message_pipe_.get(), kMessageSize,
                                     kMessages);
  tester.RunAndCheckResults();
}

TEST_F(IceTransportTest, FailedChannelAuth) {
  // Use host authenticator with one that rejects channel authentication.
  host_authenticator_.reset(
      new FakeAuthenticator(FakeAuthenticator::REJECT_CHANNEL));

  InitializeConnection();

  client_transport_->GetChannelFactory()->CreateChannel(
      kChannelName, base::Bind(&IceTransportTest::OnClientChannelCreated,
                               base::Unretained(this)));
  host_transport_->GetChannelFactory()->CreateChannel(
      kChannelName, base::Bind(&IceTransportTest::OnHostChannelCreated,
                               base::Unretained(this)));

  run_loop_.reset(new base::RunLoop());

  // The callback should never be called.
  EXPECT_CALL(host_channel_callback_, OnDone(_)).Times(0);

  run_loop_->Run();

  EXPECT_FALSE(host_message_pipe_);
  EXPECT_EQ(CHANNEL_CONNECTION_ERROR, error_);

  client_transport_->GetChannelFactory()->CancelChannelCreation(
      kChannelName);
}

// Verify that channels are never marked connected if connection cannot be
// established.
TEST_F(IceTransportTest, TestBrokenTransport) {
  // Allow only incoming connections on both ends, which effectively renders
  // transport unusable. Also reduce connection timeout so the test finishes
  // quickly.
  network_settings_ = NetworkSettings(NetworkSettings::NAT_TRAVERSAL_DISABLED);
  network_settings_.ice_timeout = base::TimeDelta::FromSeconds(1);
  network_settings_.ice_reconnect_attempts = 1;

  InitializeConnection();

  client_transport_->GetChannelFactory()->CreateChannel(
      kChannelName, base::Bind(&IceTransportTest::OnClientChannelCreated,
                               base::Unretained(this)));
  host_transport_->GetChannelFactory()->CreateChannel(
      kChannelName, base::Bind(&IceTransportTest::OnHostChannelCreated,
                               base::Unretained(this)));

  // The RunLoop should quit in OnTransportError().
  run_loop_.reset(new base::RunLoop());
  run_loop_->Run();

  // Verify that neither of the two ends of the channel is connected.
  EXPECT_FALSE(client_message_pipe_);
  EXPECT_FALSE(host_message_pipe_);
  EXPECT_EQ(CHANNEL_CONNECTION_ERROR, error_);

  client_transport_->GetChannelFactory()->CancelChannelCreation(
      kChannelName);
  host_transport_->GetChannelFactory()->CancelChannelCreation(
      kChannelName);
}

TEST_F(IceTransportTest, TestCancelChannelCreation) {
  InitializeConnection();

  client_transport_->GetChannelFactory()->CreateChannel(
      kChannelName, base::Bind(&IceTransportTest::OnClientChannelCreated,
                               base::Unretained(this)));
  client_transport_->GetChannelFactory()->CancelChannelCreation(
      kChannelName);

  EXPECT_TRUE(!client_message_pipe_.get());
}

// Verify that we can still connect even when there is a delay in signaling
// messages delivery.
TEST_F(IceTransportTest, TestDelayedSignaling) {
  transport_info_delay_ = base::TimeDelta::FromMilliseconds(100);

  InitializeConnection();

  client_transport_->GetChannelFactory()->CreateChannel(
      kChannelName, base::Bind(&IceTransportTest::OnClientChannelCreated,
                               base::Unretained(this)));
  host_transport_->GetChannelFactory()->CreateChannel(
      kChannelName, base::Bind(&IceTransportTest::OnHostChannelCreated,
                               base::Unretained(this)));

  WaitUntilConnected();

  MessagePipeConnectionTester tester(host_message_pipe_.get(),
                                     client_message_pipe_.get(), kMessageSize,
                                     kMessages);
  tester.RunAndCheckResults();
}


}  // namespace protocol
}  // namespace remoting

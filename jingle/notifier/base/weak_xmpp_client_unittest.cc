// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jingle/notifier/base/weak_xmpp_client.h"

#include <memory>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/message_loop/message_loop.h"
#include "jingle/glue/task_pump.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "webrtc/rtc_base/sigslot.h"

namespace notifier {

namespace {

class MockXmppDelegate : public sigslot::has_slots<> {
 public:
  virtual ~MockXmppDelegate() {}

  MOCK_METHOD1(OnStateChange, void(buzz::XmppEngine::State));
  MOCK_METHOD2(OnInputLog, void(const char*, int));
  MOCK_METHOD2(OnOutputLog, void(const char*, int));
};

const buzz::XmppEngine::State kState = buzz::XmppEngine::STATE_OPEN;
const char kInputLog[] = "input log";
const char kOutputLog[] = "output log";

class WeakXmppClientTest : public testing::Test {
 protected:
  WeakXmppClientTest() : task_pump_(new jingle_glue::TaskPump()) {}

  ~WeakXmppClientTest() override {}

  void ConnectSignals(buzz::XmppClient* xmpp_client) {
    xmpp_client->SignalStateChange.connect(
        &mock_xmpp_delegate_, &MockXmppDelegate::OnStateChange);
    xmpp_client->SignalLogInput.connect(
        &mock_xmpp_delegate_, &MockXmppDelegate::OnInputLog);
    xmpp_client->SignalLogOutput.connect(
        &mock_xmpp_delegate_, &MockXmppDelegate::OnOutputLog);
  }

  void ExpectSignalCalls() {
    EXPECT_CALL(mock_xmpp_delegate_, OnStateChange(kState));
    EXPECT_CALL(mock_xmpp_delegate_,
                OnInputLog(kInputLog, arraysize(kInputLog)));
    EXPECT_CALL(mock_xmpp_delegate_,
                OnOutputLog(kOutputLog, arraysize(kOutputLog)));
  }

  void RaiseSignals(buzz::XmppClient* xmpp_client) {
    xmpp_client->SignalStateChange(kState);
    xmpp_client->SignalLogInput(kInputLog, arraysize(kInputLog));
    xmpp_client->SignalLogOutput(kOutputLog, arraysize(kOutputLog));
  }

  // Needed by TaskPump.
  base::MessageLoop message_loop_;

  std::unique_ptr<jingle_glue::TaskPump> task_pump_;
  MockXmppDelegate mock_xmpp_delegate_;
};

TEST_F(WeakXmppClientTest, InvalidationViaInvalidate) {
  ExpectSignalCalls();

  WeakXmppClient* weak_xmpp_client = new WeakXmppClient(task_pump_.get());
  ConnectSignals(weak_xmpp_client);

  weak_xmpp_client->Start();
  base::WeakPtr<WeakXmppClient> weak_ptr = weak_xmpp_client->AsWeakPtr();
  EXPECT_TRUE(weak_ptr.get());
  RaiseSignals(weak_ptr.get());

  weak_xmpp_client->Invalidate();
  EXPECT_FALSE(weak_ptr.get());
  // We know that |weak_xmpp_client| is still valid at this point,
  // although it should be entirely disconnected.
  RaiseSignals(weak_xmpp_client);
}

TEST_F(WeakXmppClientTest, InvalidationViaStop) {
  ExpectSignalCalls();

  WeakXmppClient* weak_xmpp_client = new WeakXmppClient(task_pump_.get());
  ConnectSignals(weak_xmpp_client);

  weak_xmpp_client->Start();
  base::WeakPtr<WeakXmppClient> weak_ptr = weak_xmpp_client->AsWeakPtr();
  EXPECT_TRUE(weak_ptr.get());
  RaiseSignals(weak_ptr.get());

  weak_xmpp_client->Abort();
  EXPECT_FALSE(weak_ptr.get());
  // We know that |weak_xmpp_client| is still valid at this point,
  // although it should be entirely disconnected.
  RaiseSignals(weak_xmpp_client);
}

TEST_F(WeakXmppClientTest, InvalidationViaDestructor) {
  ExpectSignalCalls();

  WeakXmppClient* weak_xmpp_client = new WeakXmppClient(task_pump_.get());
  ConnectSignals(weak_xmpp_client);

  weak_xmpp_client->Start();
  base::WeakPtr<WeakXmppClient> weak_ptr = weak_xmpp_client->AsWeakPtr();
  EXPECT_TRUE(weak_ptr.get());
  RaiseSignals(weak_ptr.get());

  task_pump_.reset();
  EXPECT_FALSE(weak_ptr.get());
  // |weak_xmpp_client| is truly invalid at this point so we can't
  // RaiseSignals() with it.
}

}  // namespace

}  // namespace notifier

// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/media/peer_connection_tracker.h"

#include "base/message_loop/message_loop.h"
#include "content/common/media/peer_connection_tracker_messages.h"
#include "content/public/test/mock_render_thread.h"
#include "content/renderer/media/mock_web_rtc_peer_connection_handler_client.h"
#include "content/renderer/media/rtc_peer_connection_handler.h"
#include "content/renderer/media/webrtc/mock_peer_connection_dependency_factory.h"
#include "ipc/ipc_message_macros.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/WebKit/public/platform/WebMediaConstraints.h"
#include "third_party/WebKit/public/platform/WebRTCOfferOptions.h"

using ::testing::_;

namespace content {

namespace {

class MockSendTargetThread : public MockRenderThread {
 public:
  MOCK_METHOD3(OnUpdatePeerConnection, void(int, std::string, std::string));
  MOCK_METHOD1(OnAddPeerConnection, void(PeerConnectionInfo));

 private:
  bool OnMessageReceived(const IPC::Message& msg) override;
};

bool MockSendTargetThread::OnMessageReceived(const IPC::Message& msg) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(MockSendTargetThread, msg)
    IPC_MESSAGE_HANDLER(PeerConnectionTrackerHost_UpdatePeerConnection,
                        OnUpdatePeerConnection)
    IPC_MESSAGE_HANDLER(PeerConnectionTrackerHost_AddPeerConnection,
                        OnAddPeerConnection)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

class MockPeerConnectionHandler : public RTCPeerConnectionHandler {
 public:
  MockPeerConnectionHandler()
      : RTCPeerConnectionHandler(&client_, &dependency_factory_) {}
  MOCK_METHOD0(CloseClientPeerConnection, void());

 private:
  MockPeerConnectionDependencyFactory dependency_factory_;
  MockWebRTCPeerConnectionHandlerClient client_;
};

class PeerConnectionTrackerTest : public ::testing::Test {
 private:
  base::MessageLoop message_loop_;
};

}  // namespace

TEST_F(PeerConnectionTrackerTest, CreatingObject) {
  PeerConnectionTracker tracker;
}

TEST_F(PeerConnectionTrackerTest, TrackCreateOffer) {
  PeerConnectionTracker tracker;
  // Note: blink::WebRTCOfferOptions is not mockable. So we can't write
  // tests for anything but a null options parameter.
  blink::WebRTCOfferOptions options(0, 0, false, false);
  // Initialization stuff. This can be separated into a test class.
  MockPeerConnectionHandler pc_handler;
  MockSendTargetThread target_thread;
  webrtc::PeerConnectionInterface::RTCConfiguration config;
  blink::WebMediaConstraints constraints;
  tracker.OverrideSendTargetForTesting(&target_thread);
  EXPECT_CALL(target_thread, OnAddPeerConnection(_));
  tracker.RegisterPeerConnection(&pc_handler, config, constraints, nullptr);
  // Back to the test.
  EXPECT_CALL(target_thread,
              OnUpdatePeerConnection(
                  _, "createOffer",
                  "options: {offerToReceiveVideo: 0, offerToReceiveAudio: 0, "
                  "voiceActivityDetection: false, iceRestart: false}"));
  tracker.TrackCreateOffer(&pc_handler, options);
}

TEST_F(PeerConnectionTrackerTest, OnSuspend) {
  PeerConnectionTracker tracker;
  // Initialization stuff.
  MockPeerConnectionHandler pc_handler;
  MockSendTargetThread target_thread;
  webrtc::PeerConnectionInterface::RTCConfiguration config;
  blink::WebMediaConstraints constraints;
  tracker.OverrideSendTargetForTesting(&target_thread);
  EXPECT_CALL(target_thread, OnAddPeerConnection(_));
  tracker.RegisterPeerConnection(&pc_handler, config, constraints, nullptr);
  EXPECT_CALL(pc_handler, CloseClientPeerConnection());
  std::unique_ptr<IPC::Message> message(new PeerConnectionTracker_OnSuspend());
  tracker.OnControlMessageReceived(*message.get());
}

// TODO(hta): Write tests for the other tracking functions.

}  // namespace

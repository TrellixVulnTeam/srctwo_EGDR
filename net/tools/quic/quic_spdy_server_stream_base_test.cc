// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/tools/quic/quic_spdy_server_stream_base.h"

#include "net/quic/platform/api/quic_ptr_util.h"
#include "net/quic/platform/api/quic_test.h"
#include "net/quic/test_tools/quic_spdy_session_peer.h"
#include "net/quic/test_tools/quic_test_utils.h"

using testing::_;

namespace net {
namespace test {
namespace {

class TestQuicSpdyServerStream : public QuicSpdyServerStreamBase {
 public:
  TestQuicSpdyServerStream(QuicStreamId id, QuicSpdySession* session)
      : QuicSpdyServerStreamBase(id, session) {}

  void OnDataAvailable() override {}
};

class QuicSpdyServerStreamBaseTest : public QuicTest {
 protected:
  QuicSpdyServerStreamBaseTest()
      : session_(new MockQuicConnection(&helper_,
                                        &alarm_factory_,
                                        Perspective::IS_SERVER)) {
    stream_ = new TestQuicSpdyServerStream(
        QuicSpdySessionPeer::GetNthClientInitiatedStreamId(session_, 0),
        &session_);
    session_.ActivateStream(QuicWrapUnique(stream_));
  }

  QuicSpdyServerStreamBase* stream_ = nullptr;
  MockQuicConnectionHelper helper_;
  MockAlarmFactory alarm_factory_;
  MockQuicSpdySession session_;
};

TEST_F(QuicSpdyServerStreamBaseTest,
       SendQuicRstStreamNoErrorWithEarlyResponse) {
  stream_->StopReading();
  EXPECT_CALL(session_, SendRstStream(_, QUIC_STREAM_NO_ERROR, _)).Times(1);
  stream_->set_fin_sent(true);
  stream_->CloseWriteSide();
}

TEST_F(QuicSpdyServerStreamBaseTest,
       DoNotSendQuicRstStreamNoErrorWithRstReceived) {
  EXPECT_FALSE(stream_->reading_stopped());

  EXPECT_CALL(session_, SendRstStream(_, QUIC_STREAM_NO_ERROR, _)).Times(0);
  EXPECT_CALL(session_, SendRstStream(_, QUIC_RST_ACKNOWLEDGEMENT, _)).Times(1);
  QuicRstStreamFrame rst_frame(stream_->id(), QUIC_STREAM_CANCELLED, 1234);
  stream_->OnStreamReset(rst_frame);

  EXPECT_TRUE(stream_->reading_stopped());
  EXPECT_TRUE(stream_->write_side_closed());
}

}  // namespace
}  // namespace test
}  // namespace net

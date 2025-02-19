// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/protocol/host_video_dispatcher.h"

#include <utility>

#include "base/bind.h"
#include "net/socket/stream_socket.h"
#include "remoting/base/compound_buffer.h"
#include "remoting/base/constants.h"
#include "remoting/proto/video.pb.h"
#include "remoting/protocol/message_pipe.h"
#include "remoting/protocol/message_serialization.h"
#include "remoting/protocol/video_feedback_stub.h"

namespace remoting {
namespace protocol {

HostVideoDispatcher::HostVideoDispatcher()
    : ChannelDispatcherBase(kVideoChannelName) {}
HostVideoDispatcher::~HostVideoDispatcher() {}

void HostVideoDispatcher::ProcessVideoPacket(
    std::unique_ptr<VideoPacket> packet,
    const base::Closure& done) {
  message_pipe()->Send(packet.get(), done);
}

void HostVideoDispatcher::OnIncomingMessage(
    std::unique_ptr<CompoundBuffer> message) {
  std::unique_ptr<VideoAck> ack = ParseMessage<VideoAck>(message.get());
  if (!ack)
    return;
  if (video_feedback_stub_)
    video_feedback_stub_->ProcessVideoAck(std::move(ack));
}

}  // namespace protocol
}  // namespace remoting

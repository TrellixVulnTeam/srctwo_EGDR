// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/quic/core/congestion_control/prr_sender.h"

#include "net/quic/core/quic_packets.h"

namespace net {

namespace {
// Constant based on TCP defaults.
const QuicByteCount kMaxSegmentSize = kDefaultTCPMSS;
}  // namespace

PrrSender::PrrSender()
    : bytes_sent_since_loss_(0),
      bytes_delivered_since_loss_(0),
      ack_count_since_loss_(0),
      bytes_in_flight_before_loss_(0) {}

void PrrSender::OnPacketSent(QuicByteCount sent_bytes) {
  bytes_sent_since_loss_ += sent_bytes;
}

void PrrSender::OnPacketLost(QuicByteCount prior_in_flight) {
  bytes_sent_since_loss_ = 0;
  bytes_in_flight_before_loss_ = prior_in_flight;
  bytes_delivered_since_loss_ = 0;
  ack_count_since_loss_ = 0;
}

void PrrSender::OnPacketAcked(QuicByteCount acked_bytes) {
  bytes_delivered_since_loss_ += acked_bytes;
  ++ack_count_since_loss_;
}

QuicTime::Delta PrrSender::TimeUntilSend(
    QuicByteCount congestion_window,
    QuicByteCount bytes_in_flight,
    QuicByteCount slowstart_threshold) const {
  //   if (FLAGS_?? && bytes_in_flight < congestion_window) {
  //     return QuicTime::Delta::Zero();
  //   }
  // Return QuicTime::Zero In order to ensure limited transmit always works.
  if (bytes_sent_since_loss_ == 0 || bytes_in_flight < kMaxSegmentSize) {
    return QuicTime::Delta::Zero();
  }
  if (congestion_window > bytes_in_flight) {
    // During PRR-SSRB, limit outgoing packets to 1 extra MSS per ack, instead
    // of sending the entire available window. This prevents burst retransmits
    // when more packets are lost than the CWND reduction.
    //   limit = MAX(prr_delivered - prr_out, DeliveredData) + MSS
    if (bytes_delivered_since_loss_ + ack_count_since_loss_ * kMaxSegmentSize <=
        bytes_sent_since_loss_) {
      return QuicTime::Delta::Infinite();
    }
    return QuicTime::Delta::Zero();
  }
  // Implement Proportional Rate Reduction (RFC6937).
  // Checks a simplified version of the PRR formula that doesn't use division:
  // AvailableSendWindow =
  //   CEIL(prr_delivered * ssthresh / BytesInFlightAtLoss) - prr_sent
  if (bytes_delivered_since_loss_ * slowstart_threshold >
      bytes_sent_since_loss_ * bytes_in_flight_before_loss_) {
    return QuicTime::Delta::Zero();
  }
  return QuicTime::Delta::Infinite();
}

}  // namespace net

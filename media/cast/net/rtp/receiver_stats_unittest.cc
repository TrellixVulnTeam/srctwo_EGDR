// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <stdint.h>

#include "base/macros.h"
#include "base/test/simple_test_tick_clock.h"
#include "base/time/time.h"
#include "media/cast/constants.h"
#include "media/cast/net/rtp/receiver_stats.h"
#include "media/cast/net/rtp/rtp_defines.h"

namespace media {
namespace cast {

static const int64_t kStartMillisecond = INT64_C(12345678900000);
static const uint32_t kStdTimeIncrementMs = 33;

class ReceiverStatsTest : public ::testing::Test {
 protected:
  ReceiverStatsTest()
      : stats_(&testing_clock_) {
    testing_clock_.Advance(
        base::TimeDelta::FromMilliseconds(kStartMillisecond));
    start_time_ = testing_clock_.NowTicks();
    delta_increments_ = base::TimeDelta::FromMilliseconds(kStdTimeIncrementMs);
  }
  ~ReceiverStatsTest() override {}

  uint32_t ExpectedJitter(uint32_t const_interval, int num_packets) {
    float jitter = 0;
    // Assume timestamps have a constant kStdTimeIncrementMs interval.
    float float_interval =
        static_cast<float>(const_interval - kStdTimeIncrementMs);
    for (int i = 0; i < num_packets; ++i) {
      jitter += (float_interval - jitter) / 16;
    }
    return static_cast<uint32_t>(jitter + 0.5f);
  }

  ReceiverStats stats_;
  RtpCastHeader rtp_header_;
  base::SimpleTestTickClock testing_clock_;
  base::TimeTicks start_time_;
  base::TimeDelta delta_increments_;

 private:
  DISALLOW_COPY_AND_ASSIGN(ReceiverStatsTest);
};

TEST_F(ReceiverStatsTest, ResetState) {
  RtpReceiverStatistics s = stats_.GetStatistics();
  EXPECT_EQ(0u, s.fraction_lost);
  EXPECT_EQ(0u, s.cumulative_lost);
  EXPECT_EQ(0u, s.extended_high_sequence_number);
  EXPECT_EQ(0u, s.jitter);
}

TEST_F(ReceiverStatsTest, LossCount) {
  for (int i = 0; i < 300; ++i) {
    if (i % 4)
      stats_.UpdateStatistics(rtp_header_, kVideoFrequency);
    if (i % 3) {
      rtp_header_.rtp_timestamp += RtpTimeDelta::FromTimeDelta(
          base::TimeDelta::FromMilliseconds(33), kVideoFrequency);
    }
    ++rtp_header_.sequence_number;
    testing_clock_.Advance(delta_increments_);
  }
  RtpReceiverStatistics s = stats_.GetStatistics();
  EXPECT_EQ(63u, s.fraction_lost);
  EXPECT_EQ(74u, s.cumulative_lost);
  // Build extended sequence number.
  const uint32_t extended_seq_num = rtp_header_.sequence_number - 1;
  EXPECT_EQ(extended_seq_num, s.extended_high_sequence_number);
}

TEST_F(ReceiverStatsTest, NoLossWrap) {
  rtp_header_.sequence_number = 65500;
  for (int i = 0; i < 300; ++i) {
    stats_.UpdateStatistics(rtp_header_, kVideoFrequency);
    if (i % 3) {
      rtp_header_.rtp_timestamp += RtpTimeDelta::FromTimeDelta(
          base::TimeDelta::FromMilliseconds(33), kVideoFrequency);
    }
    ++rtp_header_.sequence_number;
    testing_clock_.Advance(delta_increments_);
  }
  RtpReceiverStatistics s = stats_.GetStatistics();
  EXPECT_EQ(0u, s.fraction_lost);
  EXPECT_EQ(0u, s.cumulative_lost);
  // Build extended sequence number (one wrap cycle).
  const uint32_t extended_seq_num = (1 << 16) + rtp_header_.sequence_number - 1;
  EXPECT_EQ(extended_seq_num, s.extended_high_sequence_number);
}

TEST_F(ReceiverStatsTest, LossCountWrap) {
  const uint32_t kStartSequenceNumber = 65500;
  rtp_header_.sequence_number = kStartSequenceNumber;
  for (int i = 0; i < 300; ++i) {
    if (i % 4)
      stats_.UpdateStatistics(rtp_header_, kVideoFrequency);
    if (i % 3)
      rtp_header_.rtp_timestamp += RtpTimeDelta::FromTicks(1);
    ++rtp_header_.sequence_number;
    testing_clock_.Advance(delta_increments_);
  }
  RtpReceiverStatistics s = stats_.GetStatistics();
  EXPECT_EQ(63u, s.fraction_lost);
  EXPECT_EQ(74u, s.cumulative_lost);
  // Build extended sequence number (one wrap cycle).
  const uint32_t extended_seq_num = (1 << 16) + rtp_header_.sequence_number - 1;
  EXPECT_EQ(extended_seq_num, s.extended_high_sequence_number);
}

TEST_F(ReceiverStatsTest, BasicJitter) {
  for (int i = 0; i < 300; ++i) {
    stats_.UpdateStatistics(rtp_header_, kVideoFrequency);
    ++rtp_header_.sequence_number;
    rtp_header_.rtp_timestamp += RtpTimeDelta::FromTimeDelta(
        base::TimeDelta::FromMilliseconds(33), kVideoFrequency);
    testing_clock_.Advance(delta_increments_);
  }
  RtpReceiverStatistics s = stats_.GetStatistics();
  EXPECT_FALSE(s.fraction_lost);
  EXPECT_FALSE(s.cumulative_lost);
  // Build extended sequence number (one wrap cycle).
  const uint32_t extended_seq_num = rtp_header_.sequence_number - 1;
  EXPECT_EQ(extended_seq_num, s.extended_high_sequence_number);
  EXPECT_EQ(ExpectedJitter(kStdTimeIncrementMs, 300), s.jitter);
}

TEST_F(ReceiverStatsTest, NonTrivialJitter) {
  const int kAdditionalIncrement = 5;
  for (int i = 0; i < 300; ++i) {
    stats_.UpdateStatistics(rtp_header_, kVideoFrequency);
    ++rtp_header_.sequence_number;
    rtp_header_.rtp_timestamp += RtpTimeDelta::FromTimeDelta(
        base::TimeDelta::FromMilliseconds(33), kVideoFrequency);
    base::TimeDelta additional_delta =
        base::TimeDelta::FromMilliseconds(kAdditionalIncrement);
    testing_clock_.Advance(delta_increments_ + additional_delta);
  }
  RtpReceiverStatistics s = stats_.GetStatistics();
  EXPECT_FALSE(s.fraction_lost);
  EXPECT_FALSE(s.cumulative_lost);
  // Build extended sequence number (one wrap cycle).
  const uint32_t extended_seq_num = rtp_header_.sequence_number - 1;
  EXPECT_EQ(extended_seq_num, s.extended_high_sequence_number);
  EXPECT_EQ(ExpectedJitter(kStdTimeIncrementMs + kAdditionalIncrement, 300),
            s.jitter);
}

}  // namespace cast
}  // namespace media

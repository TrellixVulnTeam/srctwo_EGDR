// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "components/viz/common/frame_sinks/begin_frame_args.h"
#include "components/viz/test/begin_frame_args_test.h"
#include "testing/gtest/include/gtest/gtest-spi.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace viz {
namespace {

constexpr base::TimeDelta k1Usec = base::TimeDelta::FromMicroseconds(1);
constexpr base::TimeDelta k2Usec = base::TimeDelta::FromMicroseconds(2);
constexpr base::TimeDelta k3Usec = base::TimeDelta::FromMicroseconds(3);

TEST(BeginFrameArgsTest, Helpers) {
  // Quick create methods work
  BeginFrameArgs args0 =
      CreateBeginFrameArgsForTesting(BEGINFRAME_FROM_HERE, 0, 1);
  EXPECT_TRUE(args0.IsValid()) << args0;

  BeginFrameArgs args1 =
      CreateBeginFrameArgsForTesting(BEGINFRAME_FROM_HERE, 0, 1, 0, 0, -1);
  EXPECT_FALSE(args1.IsValid()) << args1;

  BeginFrameArgs args2 =
      CreateBeginFrameArgsForTesting(BEGINFRAME_FROM_HERE, 123, 10, 1, 2, 3);
  EXPECT_TRUE(args2.IsValid()) << args2;
  EXPECT_EQ(123u, args2.source_id);
  EXPECT_EQ(10u, args2.sequence_number);
  EXPECT_EQ(k1Usec, args2.frame_time.since_origin());
  EXPECT_EQ(k2Usec, args2.deadline.since_origin());
  EXPECT_EQ(k3Usec, args2.interval);
  EXPECT_EQ(BeginFrameArgs::NORMAL, args2.type);

  BeginFrameArgs args4 = CreateBeginFrameArgsForTesting(
      BEGINFRAME_FROM_HERE, 234, 20, 1, 2, 3, BeginFrameArgs::MISSED);
  EXPECT_TRUE(args4.IsValid()) << args4;
  EXPECT_EQ(234u, args4.source_id);
  EXPECT_EQ(20u, args4.sequence_number);
  EXPECT_EQ(k1Usec, args4.frame_time.since_origin());
  EXPECT_EQ(k2Usec, args4.deadline.since_origin());
  EXPECT_EQ(k3Usec, args4.interval);
  EXPECT_EQ(BeginFrameArgs::MISSED, args4.type);

  // operator==
  EXPECT_EQ(
      CreateBeginFrameArgsForTesting(BEGINFRAME_FROM_HERE, 123, 20, 4, 5, 6),
      CreateBeginFrameArgsForTesting(BEGINFRAME_FROM_HERE, 123, 20, 4, 5, 6));

  EXPECT_NONFATAL_FAILURE(
      EXPECT_EQ(CreateBeginFrameArgsForTesting(BEGINFRAME_FROM_HERE, 123, 30, 7,
                                               8, 9, BeginFrameArgs::MISSED),
                CreateBeginFrameArgsForTesting(BEGINFRAME_FROM_HERE, 123, 30, 7,
                                               8, 9)),
      "");

  EXPECT_NONFATAL_FAILURE(
      EXPECT_EQ(CreateBeginFrameArgsForTesting(BEGINFRAME_FROM_HERE, 123, 30, 4,
                                               5, 6),
                CreateBeginFrameArgsForTesting(BEGINFRAME_FROM_HERE, 123, 30, 7,
                                               8, 9)),
      "");

  EXPECT_NONFATAL_FAILURE(
      EXPECT_EQ(CreateBeginFrameArgsForTesting(BEGINFRAME_FROM_HERE, 123, 30, 7,
                                               8, 9),
                CreateBeginFrameArgsForTesting(BEGINFRAME_FROM_HERE, 123, 40, 7,
                                               8, 9)),
      "");

  EXPECT_NONFATAL_FAILURE(
      EXPECT_EQ(CreateBeginFrameArgsForTesting(BEGINFRAME_FROM_HERE, 123, 30, 7,
                                               8, 9),
                CreateBeginFrameArgsForTesting(BEGINFRAME_FROM_HERE, 234, 30, 7,
                                               8, 9)),
      "");

  // operator<<
  std::stringstream out1;
  out1 << args1;
  EXPECT_EQ("BeginFrameArgs(NORMAL, 0, 1, 0, 0, -1us)", out1.str());
  std::stringstream out2;
  out2 << args2;
  EXPECT_EQ("BeginFrameArgs(NORMAL, 123, 10, 1, 2, 3us)", out2.str());

  // PrintTo
  EXPECT_EQ(std::string("BeginFrameArgs(NORMAL, 0, 1, 0, 0, -1us)"),
            ::testing::PrintToString(args1));
  EXPECT_EQ(std::string("BeginFrameArgs(NORMAL, 123, 10, 1, 2, 3us)"),
            ::testing::PrintToString(args2));
}

TEST(BeginFrameArgsTest, Create) {
  // BeginFrames are not valid by default
  BeginFrameArgs args1;
  EXPECT_FALSE(args1.IsValid()) << args1;
  EXPECT_TRUE(args1.on_critical_path);

  BeginFrameArgs args2 = BeginFrameArgs::Create(
      BEGINFRAME_FROM_HERE, 123, 10, base::TimeTicks() + k1Usec,
      base::TimeTicks() + k2Usec, k3Usec, BeginFrameArgs::NORMAL);
  EXPECT_TRUE(args2.IsValid()) << args2;
  EXPECT_EQ(123u, args2.source_id) << args2;
  EXPECT_EQ(10u, args2.sequence_number) << args2;
  EXPECT_EQ(k1Usec, args2.frame_time.since_origin()) << args2;
  EXPECT_EQ(k2Usec, args2.deadline.since_origin()) << args2;
  EXPECT_EQ(k3Usec, args2.interval) << args2;
  EXPECT_EQ(BeginFrameArgs::NORMAL, args2.type) << args2;
}

#ifndef NDEBUG
TEST(BeginFrameArgsTest, Location) {
  tracked_objects::Location expected_location = BEGINFRAME_FROM_HERE;

  BeginFrameArgs args = CreateBeginFrameArgsForTesting(expected_location, 0, 1);
  EXPECT_EQ(expected_location.ToString(), args.created_from.ToString());
}
#endif

}  // namespace
}  // namespace viz

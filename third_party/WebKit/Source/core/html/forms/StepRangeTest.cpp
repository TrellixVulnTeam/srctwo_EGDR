// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/html/forms/StepRange.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

TEST(StepRangeTest, ClampValueWithOutStepMatchedValue) {
  // <input type=range value=200 min=0 max=100 step=1000>
  StepRange step_range(Decimal(200), Decimal(0), Decimal(100), true,
                       Decimal(1000), StepRange::StepDescription());

  EXPECT_EQ(Decimal(100), step_range.ClampValue(Decimal(200)));
  EXPECT_EQ(Decimal(0), step_range.ClampValue(Decimal(-100)));
}

TEST(StepRangeTest, StepSnappedMaximum) {
  // <input type=number value="1110" max=100 step="20">
  StepRange step_range(Decimal::FromDouble(1110), Decimal(0), Decimal(100),
                       true, Decimal(20), StepRange::StepDescription());
  EXPECT_EQ(Decimal(90), step_range.StepSnappedMaximum());

  // crbug.com/617809
  // <input type=number
  // value="8624024784918570374158793713225864658725102756338798521486349461900449498315865014065406918592181034633618363349807887404915072776534917803019477033072906290735591367789665757384135591225430117374220731087966"
  // min=0 max=100 step="18446744073709551575">
  StepRange step_range2(Decimal::FromDouble(8.62402e+207), Decimal(0),
                        Decimal(100), true, Decimal::FromDouble(1.84467e+19),
                        StepRange::StepDescription());
  EXPECT_FALSE(step_range2.StepSnappedMaximum().IsFinite());
}

}  // namespace blink

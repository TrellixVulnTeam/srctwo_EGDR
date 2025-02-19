// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/platform/WebScreenInfo.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

TEST(WebScreenInfoTest, Equality) {
  WebScreenInfo screen_info1;
  WebScreenInfo screen_info2;

  EXPECT_EQ(screen_info1, screen_info2);

  // Change same values in screenInfo1.
  screen_info1.device_scale_factor = 10.f;
  screen_info1.depth = 3;
  screen_info1.depth_per_component = 2;
  screen_info1.is_monochrome = false;

  EXPECT_NE(screen_info1, screen_info2);

  // Set the same values to screenInfo2, they should be equal now.
  screen_info2.device_scale_factor = 10.f;
  screen_info2.depth = 3;
  screen_info2.depth_per_component = 2;
  screen_info2.is_monochrome = false;

  EXPECT_EQ(screen_info1, screen_info2);

  // Set all the known members.
  screen_info1.device_scale_factor = 2.f;
  screen_info1.depth = 1;
  screen_info1.depth_per_component = 1;
  screen_info1.is_monochrome = false;
  screen_info1.rect = WebRect(0, 0, 1024, 1024);
  screen_info1.available_rect = WebRect(0, 0, 1024, 1024);
  screen_info1.orientation_type = blink::kWebScreenOrientationLandscapePrimary;
  screen_info1.orientation_angle = 90;

  EXPECT_NE(screen_info1, screen_info2);

  screen_info2.device_scale_factor = 2.f;
  screen_info2.depth = 1;
  screen_info2.depth_per_component = 1;
  screen_info2.is_monochrome = false;
  screen_info2.rect = WebRect(0, 0, 1024, 1024);
  screen_info2.available_rect = WebRect(0, 0, 1024, 1024);
  screen_info2.orientation_type = blink::kWebScreenOrientationLandscapePrimary;
  screen_info2.orientation_angle = 90;

  EXPECT_EQ(screen_info1, screen_info2);
}

}  // namespace blink

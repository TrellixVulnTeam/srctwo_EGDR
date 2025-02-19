// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromecast/system/reboot/reboot_util.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace chromecast {

TEST(RebootUtil, SingleSetGetLastRebootSource) {
  if (RebootUtil::SetLastRebootSource(RebootShlib::RebootSource::FORCED)) {
    EXPECT_EQ(RebootUtil::GetLastRebootSource(),
              RebootShlib::RebootSource::FORCED);
  }
}

TEST(RebootUtil, MultipleSetGetLastRebootSource) {
  if (RebootUtil::SetLastRebootSource(RebootShlib::RebootSource::FORCED)) {
    EXPECT_EQ(RebootUtil::GetLastRebootSource(),
              RebootShlib::RebootSource::FORCED);
  }

  if (RebootUtil::SetLastRebootSource(RebootShlib::RebootSource::OTA)) {
    EXPECT_EQ(RebootUtil::GetLastRebootSource(),
              RebootShlib::RebootSource::OTA);
  }

  if (RebootUtil::SetLastRebootSource(RebootShlib::RebootSource::FDR)) {
    EXPECT_EQ(RebootUtil::GetLastRebootSource(),
              RebootShlib::RebootSource::FDR);
  }
}

}  // namespace chromecast

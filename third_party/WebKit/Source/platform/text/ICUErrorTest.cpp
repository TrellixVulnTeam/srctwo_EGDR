// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/text/ICUError.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

const UErrorCode kTestErrorCode = U_INVALID_FORMAT_ERROR;

void CauseICUError(UErrorCode& err) {
  err = kTestErrorCode;
}

TEST(ICUErrorTest, assignToAutomaticReference) {
  ICUError icu_error;
  EXPECT_EQ(icu_error, U_ZERO_ERROR);
  CauseICUError(icu_error);
  EXPECT_EQ(icu_error, kTestErrorCode);
}

}  // namespace blink

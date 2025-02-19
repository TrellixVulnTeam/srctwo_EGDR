// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/common/referrer.h"

#include "base/test/gtest_util.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace content {

using ReferrerSanitizerTest = testing::Test;

TEST_F(ReferrerSanitizerTest, SanitizesPolicyForEmptyReferrers) {
  EXPECT_DCHECK_DEATH(ignore_result(Referrer::SanitizeForRequest(
      GURL("https://a"),
      Referrer(GURL(), static_cast<blink::WebReferrerPolicy>(200)))));
}

TEST_F(ReferrerSanitizerTest, SanitizesPolicyForNonEmptyReferrers) {
  EXPECT_DCHECK_DEATH(ignore_result(Referrer::SanitizeForRequest(
      GURL("https://a"),
      Referrer(GURL("http://b"), static_cast<blink::WebReferrerPolicy>(200)))));
}

}  // namespace content

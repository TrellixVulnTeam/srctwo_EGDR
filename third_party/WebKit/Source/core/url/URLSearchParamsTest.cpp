// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/url/URLSearchParams.h"

#include <gtest/gtest.h>

namespace blink {

using URLSearchParamsTest = ::testing::Test;

TEST_F(URLSearchParamsTest, ToEncodedFormData) {
  URLSearchParams* params = URLSearchParams::Create(String());
  EXPECT_EQ("", params->ToEncodedFormData()->FlattenToString());

  params->append("name", "value");
  EXPECT_EQ("name=value", params->ToEncodedFormData()->FlattenToString());

  params->append("another name", "another value");
  EXPECT_EQ("name=value&another+name=another+value",
            params->ToEncodedFormData()->FlattenToString());
}

}  // namespace blink

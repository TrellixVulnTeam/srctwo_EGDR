// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/loader/fetch/FetchUtils.h"

#include "platform/wtf/text/WTFString.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

namespace {

TEST(FetchUtilsTest, NormalizeHeaderValue) {
  EXPECT_EQ("t", FetchUtils::NormalizeHeaderValue(" t"));
  EXPECT_EQ("t", FetchUtils::NormalizeHeaderValue("t "));
  EXPECT_EQ("t", FetchUtils::NormalizeHeaderValue(" t "));
  EXPECT_EQ("test", FetchUtils::NormalizeHeaderValue("test\r"));
  EXPECT_EQ("test", FetchUtils::NormalizeHeaderValue("test\n"));
  EXPECT_EQ("test", FetchUtils::NormalizeHeaderValue("test\r\n"));
  EXPECT_EQ("test", FetchUtils::NormalizeHeaderValue("test\t"));
  EXPECT_EQ("t t", FetchUtils::NormalizeHeaderValue("t t"));
  EXPECT_EQ("t\tt", FetchUtils::NormalizeHeaderValue("t\tt"));
  EXPECT_EQ("t\rt", FetchUtils::NormalizeHeaderValue("t\rt"));
  EXPECT_EQ("t\nt", FetchUtils::NormalizeHeaderValue("t\nt"));
  EXPECT_EQ("t\r\nt", FetchUtils::NormalizeHeaderValue("t\r\nt"));
  EXPECT_EQ("test", FetchUtils::NormalizeHeaderValue("\rtest"));
  EXPECT_EQ("test", FetchUtils::NormalizeHeaderValue("\ntest"));
  EXPECT_EQ("test", FetchUtils::NormalizeHeaderValue("\r\ntest"));
  EXPECT_EQ("test", FetchUtils::NormalizeHeaderValue("\ttest"));
  EXPECT_EQ("", FetchUtils::NormalizeHeaderValue(""));
  EXPECT_EQ("", FetchUtils::NormalizeHeaderValue(" "));
  EXPECT_EQ("", FetchUtils::NormalizeHeaderValue("\r\n\r\n\r\n"));
  EXPECT_EQ("\xd0\xa1", FetchUtils::NormalizeHeaderValue("\xd0\xa1"));
  EXPECT_EQ("test", FetchUtils::NormalizeHeaderValue("test"));
}

}  // namespace

}  // namespace blink

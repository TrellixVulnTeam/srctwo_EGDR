// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/http2/hpack/hpack_string.h"

// Tests of HpackString.

#include <utility>

#include "base/logging.h"
#include "net/http2/tools/failure.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::AssertionFailure;
using ::testing::AssertionResult;
using ::testing::AssertionSuccess;

namespace net {
namespace test {
namespace {

const char kStr0[] = "s0: Some string to be copied into another string.";
const char kStr1[] = "S1 - some string to be copied into yet another string.";

class HpackStringTest : public ::testing::Test {
 protected:
  AssertionResult VerifyNotEqual(HpackString* actual,
                                 const Http2String& not_expected_str) {
    const char* not_expected_ptr = not_expected_str.c_str();
    Http2StringPiece not_expected_sp(not_expected_str);

    VERIFY_NE(*actual, not_expected_ptr);
    VERIFY_NE(*actual, not_expected_sp);
    VERIFY_NE(*actual, not_expected_str);
    VERIFY_NE(actual->ToStringPiece(), not_expected_sp);

    if (!(not_expected_ptr != *actual)) {
      return AssertionFailure();
    }
    if (!(not_expected_sp != *actual)) {
      return AssertionFailure();
    }
    if (!(not_expected_str != *actual)) {
      return AssertionFailure();
    }
    if (!(not_expected_sp != actual->ToStringPiece())) {
      return AssertionFailure();
    }

    return AssertionSuccess();
  }

  AssertionResult VerifyEqual(HpackString* actual,
                              const Http2String& expected_str) {
    VERIFY_EQ(actual->size(), expected_str.size());

    const char* expected_ptr = expected_str.c_str();
    const Http2StringPiece expected_sp(expected_str);

    VERIFY_EQ(*actual, expected_ptr);
    VERIFY_EQ(*actual, expected_sp);
    VERIFY_EQ(*actual, expected_str);
    VERIFY_EQ(actual->ToStringPiece(), expected_sp);

    if (!(expected_sp == *actual)) {
      return AssertionFailure();
    }
    if (!(expected_ptr == *actual)) {
      return AssertionFailure();
    }
    if (!(expected_str == *actual)) {
      return AssertionFailure();
    }
    if (!(expected_sp == actual->ToStringPiece())) {
      return AssertionFailure();
    }

    return AssertionSuccess();
  }
};

TEST_F(HpackStringTest, CharArrayConstructor) {
  HpackString hs0(kStr0);
  EXPECT_TRUE(VerifyEqual(&hs0, kStr0));
  EXPECT_TRUE(VerifyNotEqual(&hs0, kStr1));

  HpackString hs1(kStr1);
  EXPECT_TRUE(VerifyEqual(&hs1, kStr1));
  EXPECT_TRUE(VerifyNotEqual(&hs1, kStr0));
}

TEST_F(HpackStringTest, StringPieceConstructor) {
  Http2StringPiece sp0(kStr0);
  HpackString hs0(sp0);
  EXPECT_TRUE(VerifyEqual(&hs0, kStr0));
  EXPECT_TRUE(VerifyNotEqual(&hs0, kStr1));

  Http2StringPiece sp1(kStr1);
  HpackString hs1(sp1);
  EXPECT_TRUE(VerifyEqual(&hs1, kStr1));
  EXPECT_TRUE(VerifyNotEqual(&hs1, kStr0));
}

TEST_F(HpackStringTest, MoveStringConstructor) {
  Http2String str0(kStr0);
  HpackString hs0(str0);
  EXPECT_TRUE(VerifyEqual(&hs0, kStr0));
  EXPECT_TRUE(VerifyNotEqual(&hs0, kStr1));

  Http2String str1(kStr1);
  HpackString hs1(str1);
  EXPECT_TRUE(VerifyEqual(&hs1, kStr1));
  EXPECT_TRUE(VerifyNotEqual(&hs1, kStr0));
}

TEST_F(HpackStringTest, CopyConstructor) {
  Http2StringPiece sp0(kStr0);
  HpackString hs0(sp0);
  HpackString hs1(hs0);
  EXPECT_EQ(hs0, hs1);

  EXPECT_TRUE(VerifyEqual(&hs0, kStr0));
  EXPECT_TRUE(VerifyEqual(&hs1, kStr0));

  EXPECT_TRUE(VerifyNotEqual(&hs0, kStr1));
  EXPECT_TRUE(VerifyNotEqual(&hs1, kStr1));
}

TEST_F(HpackStringTest, MoveConstructor) {
  Http2StringPiece sp0(kStr0);
  HpackString hs0(sp0);
  EXPECT_TRUE(VerifyEqual(&hs0, kStr0));
  EXPECT_TRUE(VerifyNotEqual(&hs0, ""));

  HpackString hs1(std::move(hs0));
  EXPECT_NE(hs0, hs1);

  EXPECT_TRUE(VerifyEqual(&hs1, kStr0));
  EXPECT_TRUE(VerifyEqual(&hs0, ""));
  EXPECT_TRUE(VerifyNotEqual(&hs1, ""));

  LOG(INFO) << hs0;
  LOG(INFO) << hs1;
}

}  // namespace
}  // namespace test
}  // namespace net

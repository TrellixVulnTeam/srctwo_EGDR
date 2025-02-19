// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/html/parser/HTMLEntityParser.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

TEST(HTMLEntityParserTest, ConsumeHTMLEntityIncomplete) {
  String original("am");  // Incomplete by purpose.
  SegmentedString src(original);

  DecodedHTMLEntity entity;
  bool not_enough_characters = false;
  bool success = ConsumeHTMLEntity(src, entity, not_enough_characters);
  EXPECT_TRUE(not_enough_characters);
  EXPECT_FALSE(success);

  // consumeHTMLEntity should recover the original SegmentedString state if
  // failed.
  EXPECT_EQ(original, src.ToString());
}

}  // namespace blink

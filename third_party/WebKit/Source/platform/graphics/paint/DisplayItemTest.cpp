// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/graphics/paint/DisplayItem.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace blink {
namespace {

#ifndef NDEBUG
TEST(DisplayItemTest, DebugStringsExist) {
  for (int type = 0; type <= DisplayItem::kTypeLast; type++) {
    String debug_string =
        DisplayItem::TypeAsDebugString(static_cast<DisplayItem::Type>(type));
    EXPECT_FALSE(debug_string.IsEmpty());
    EXPECT_NE("Unknown", debug_string);
  }
}
#endif  // NDEBUG

}  // namespace
}  // namespace blink

/*
 * Copyright (C) 2014 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "platform/text/SegmentedString.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

TEST(SegmentedStringTest, CurrentChar) {
  SegmentedString original(String("cde"));
  {
    SegmentedString copied(original);
    SegmentedString assigned;
    assigned = original;
    EXPECT_EQ("cde", original.ToString());
    EXPECT_EQ('c', original.CurrentChar());
    EXPECT_EQ('c', copied.CurrentChar());
    EXPECT_EQ('c', assigned.CurrentChar());
  }
  original.Push('b');
  {
    SegmentedString copied(original);
    SegmentedString assigned;
    assigned = original;
    EXPECT_EQ("bcde", original.ToString());
    EXPECT_EQ('b', original.CurrentChar());
    EXPECT_EQ('b', copied.CurrentChar());
    EXPECT_EQ('b', assigned.CurrentChar());
  }
  original.Push('a');
  {
    SegmentedString copied(original);
    SegmentedString assigned;
    assigned = original;
    EXPECT_EQ("abcde", original.ToString());
    EXPECT_EQ('a', original.CurrentChar());
    EXPECT_EQ('a', copied.CurrentChar());
    EXPECT_EQ('a', assigned.CurrentChar());
  }
  original.Advance();
  {
    SegmentedString copied(original);
    SegmentedString assigned;
    assigned = original;
    EXPECT_EQ("bcde", original.ToString());
    EXPECT_EQ('b', original.CurrentChar());
    EXPECT_EQ('b', copied.CurrentChar());
    EXPECT_EQ('b', assigned.CurrentChar());
  }
}

}  // namespace blink

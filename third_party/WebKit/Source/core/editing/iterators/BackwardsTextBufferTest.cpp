// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/editing/iterators/BackwardsTextBuffer.h"

#include "core/editing/EditingTestBase.h"

namespace blink {

class BackwardsTextBufferTest : public EditingTestBase {};

TEST_F(BackwardsTextBufferTest, pushCharacters) {
  BackwardsTextBuffer buffer;

  // Basic tests.
  buffer.PushCharacters('a', 1);
  buffer.PushCharacters(1u, 0);
  buffer.PushCharacters('#', 2);
  buffer.PushCharacters('\0', 1);
  EXPECT_EQ('\0', buffer[0]);
  EXPECT_EQ('#', buffer[1]);
  EXPECT_EQ('#', buffer[2]);
  EXPECT_EQ('a', buffer[3]);

  // Tests with buffer reallocation.
  buffer.PushCharacters('A', 4096);
  EXPECT_EQ('A', buffer[0]);
  EXPECT_EQ('A', buffer[4095]);
  EXPECT_EQ('\0', buffer[4096]);
  EXPECT_EQ('#', buffer[4097]);
  EXPECT_EQ('#', buffer[4098]);
  EXPECT_EQ('a', buffer[4099]);
}

TEST_F(BackwardsTextBufferTest, pushRange) {
  BackwardsTextBuffer buffer;

  // Basic tests.
  buffer.PushRange("ababc", 1);
  buffer.PushRange((UChar*)nullptr, 0);
  buffer.PushRange("#@", 2);
  UChar ch = 'x';
  buffer.PushRange(&ch, 1);
  EXPECT_EQ('x', buffer[0]);
  EXPECT_EQ('#', buffer[1]);
  EXPECT_EQ('@', buffer[2]);
  EXPECT_EQ('a', buffer[3]);

  // Tests with buffer reallocation.
  Vector<UChar> chunk(4096);
  for (unsigned i = 0; i < chunk.size(); ++i)
    chunk[i] = i % 256;
  buffer.PushRange(chunk.data(), chunk.size());
  EXPECT_EQ(0, buffer[0]);
  EXPECT_EQ(1111 % 256, buffer[1111]);
  EXPECT_EQ(255, buffer[4095]);
  EXPECT_EQ('x', buffer[4096]);
  EXPECT_EQ('#', buffer[4097]);
  EXPECT_EQ('@', buffer[4098]);
  EXPECT_EQ('a', buffer[4099]);
}

}  // namespace blink

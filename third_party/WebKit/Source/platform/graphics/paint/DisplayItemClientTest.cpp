// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/graphics/paint/DisplayItemClient.h"

#include "platform/testing/FakeDisplayItemClient.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace blink {
namespace {

#if DCHECK_IS_ON() && !defined(UNDEFINED_SANITIZER)

TEST(DisplayItemClientTest, IsAlive) {
  EXPECT_FALSE(reinterpret_cast<DisplayItemClient*>(0x12345678)->IsAlive());
  FakeDisplayItemClient* test_client = new FakeDisplayItemClient;
  EXPECT_TRUE(test_client->IsAlive());
  delete test_client;
  EXPECT_FALSE(test_client->IsAlive());
}

#endif

}  // namespace
}  // namespace blink

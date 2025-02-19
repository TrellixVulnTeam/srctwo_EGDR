// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/variations/metrics_util.h"

#include <stddef.h>
#include <stdint.h>

#include "base/macros.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace metrics {

TEST(MetricsUtilTest, HashName) {
  // Checks that hashing is stable on all platforms.
  struct {
    const char* name;
    uint32_t hash_value;
  } known_hashes[] = {
    {"a", 937752454u},
    {"1", 723085877u},
    {"Trial Name", 2713117220u},
    {"Group Name", 3201815843u},
    {"My Favorite Experiment", 3722155194u},
    {"My Awesome Group Name", 4109503236u},
    {"abcdefghijklmonpqrstuvwxyz", 787728696u},
    {"0123456789ABCDEF", 348858318U}
  };

  for (size_t i = 0; i < arraysize(known_hashes); ++i)
    EXPECT_EQ(known_hashes[i].hash_value, HashName(known_hashes[i].name));
}

}  // namespace metrics

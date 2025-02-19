// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "rlz/lib/machine_id.h"

#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "rlz/test/rlz_test_helpers.h"
#include "testing/gtest/include/gtest/gtest.h"

// This test will fail if the behavior of GetMachineId changes.
TEST(MachineDealCodeTestMachineId, MachineId) {
  base::string16 computer_sid(base::UTF8ToUTF16(
        "S-1-5-21-2345599882-2448789067-1921365677"));
  std::string id;
  rlz_lib::testing::GetMachineIdImpl(computer_sid, -1643738288, &id);
  EXPECT_STREQ("A341BA986A7E86840688977FCF20C86E253F00919E068B50F8",
               id.c_str());
}

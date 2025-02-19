// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/active_use_util.h"

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "build/build_config.h"
#include "chrome/common/chrome_switches.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(ShouldRecordActiveUse, OrdinaryCommand) {
  base::CommandLine cmd_line(base::FilePath(FILE_PATH_LITERAL("foo.exe")));
#if !defined(OS_WIN) || defined(GOOGLE_CHROME_BUILD)
  EXPECT_TRUE(ShouldRecordActiveUse(cmd_line));
#else
  EXPECT_FALSE(ShouldRecordActiveUse(cmd_line));
#endif
}

TEST(ShouldRecordActiveUse, TryChromeAgainCommand) {
  base::CommandLine cmd_line(base::FilePath(FILE_PATH_LITERAL("foo.exe")));
  cmd_line.AppendSwitchASCII(switches::kTryChromeAgain, "0");

  EXPECT_FALSE(ShouldRecordActiveUse(cmd_line));
}

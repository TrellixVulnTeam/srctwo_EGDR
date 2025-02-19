// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "chrome/browser/extensions/extension_apitest.h"

namespace {
// This should be consistent with
// chrome/test/data/extensions/api_test/command_line/basics/test.js.
const char kTestCommandLineSwitch[] = "command-line-private-api-test-foo";
}  // namespace

class CommandLinePrivateApiTest : public ExtensionApiTest {
  void SetUpCommandLine(base::CommandLine* command_line) override {
    ExtensionApiTest::SetUpCommandLine(command_line);
    command_line->AppendSwitch(kTestCommandLineSwitch);
  }
};

IN_PROC_BROWSER_TEST_F(CommandLinePrivateApiTest, Basics) {
  EXPECT_TRUE(RunComponentExtensionTest("command_line/basics")) << message_;
}

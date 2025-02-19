// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/testing/sim/SimRequest.h"
#include "core/testing/sim/SimTest.h"
#include "platform/testing/UnitTestHelpers.h"

namespace blink {

using WindowProxyTest = SimTest;

// Tests that a WindowProxy is reinitialized after a navigation, even if the new
// Document does not use any scripting.
TEST_F(WindowProxyTest, ReinitializedAfterNavigation) {
  // TODO(dcheng): It's nicer to use TestingPlatformSupportWithMockScheduler,
  // but that leads to random DCHECKs in loading code.

  SimRequest main_resource("https://example.com/index.html", "text/html");
  LoadURL("https://example.com/index.html");
  main_resource.Complete(
      "<!DOCTYPE html>"
      "<html><head><script>"
      "var childWindow;"
      "function runTest() {"
      "  childWindow = window[0];"
      "  document.querySelector('iframe').onload = runTest2;"
      "  childWindow.location = 'data:text/plain,Initial.';"
      "}"
      "function runTest2() {"
      "  try {"
      "    childWindow.location = 'data:text/plain,Final.';"
      "    console.log('PASSED');"
      "  } catch (e) {"
      "    console.log('FAILED');"
      "  }"
      "  document.querySelector('iframe').onload = null;"
      "}"
      "</script></head><body onload='runTest()'>"
      "<iframe></iframe></body></html>");

  // Wait for the first data: URL to load
  testing::RunPendingTasks();

  // Wait for the second data: URL to load.
  testing::RunPendingTasks();

  ASSERT_GT(ConsoleMessages().size(), 0U);
  EXPECT_EQ("PASSED", ConsoleMessages()[0]);
}
}  // namespace blink

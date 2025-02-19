// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/extension_apitest.h"
#include "chrome/common/chrome_switches.h"
#include "net/dns/mock_host_resolver.h"

IN_PROC_BROWSER_TEST_F(ExtensionApiTest, ContentSecurityPolicy) {
  ASSERT_TRUE(StartEmbeddedTestServer());
  ASSERT_TRUE(RunExtensionTest("content_security_policy")) << message_;
}

IN_PROC_BROWSER_TEST_F(ExtensionApiTest, DefaultContentSecurityPolicy) {
  ASSERT_TRUE(StartEmbeddedTestServer());
  ASSERT_TRUE(RunExtensionTest("default_content_security_policy")) <<
      message_;
}

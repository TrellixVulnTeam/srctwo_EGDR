// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "chrome/browser/extensions/extension_apitest.h"
#include "chrome/test/base/ui_test_utils.h"

namespace extensions {

// Times out on win syzyasan, http://crbug.com/166026
#if defined(SYZYASAN)
#define MAYBE_Cookies DISABLED_Cookies
#else
#define MAYBE_Cookies Cookies
#endif
IN_PROC_BROWSER_TEST_F(ExtensionApiTest, MAYBE_Cookies) {
  ASSERT_TRUE(RunExtensionTest("cookies/api")) << message_;
}

IN_PROC_BROWSER_TEST_F(ExtensionApiTest, CookiesEvents) {
  ASSERT_TRUE(RunExtensionTest("cookies/events")) << message_;
}

IN_PROC_BROWSER_TEST_F(ExtensionApiTest, CookiesEventsSpanning) {
  // We need to initialize an incognito mode window in order have an initialized
  // incognito cookie store. Otherwise, the chrome.cookies.set operation is just
  // ignored and we won't be notified about a newly set cookie for which we want
  // to test whether the storeId is set correctly.
  OpenURLOffTheRecord(browser()->profile(), GURL("chrome://newtab/"));
  ASSERT_TRUE(RunExtensionTestIncognito("cookies/events_spanning")) << message_;
}

IN_PROC_BROWSER_TEST_F(ExtensionApiTest, CookiesNoPermission) {
  ASSERT_TRUE(RunExtensionTest("cookies/no_permission")) << message_;
}

}  // namespace extensions

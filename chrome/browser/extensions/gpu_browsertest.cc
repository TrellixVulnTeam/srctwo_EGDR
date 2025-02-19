// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/extension_browsertest.h"
#include "chrome/browser/ui/browser.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/common/web_preferences.h"
#include "extensions/browser/extension_host.h"
#include "extensions/browser/process_manager.h"

// Tests that background pages are marked as never visible to prevent GPU
// resource allocation. See crbug.com/362165 and crbug.com/163698.
IN_PROC_BROWSER_TEST_F(ExtensionBrowserTest, BackgroundPageIsNeverVisible) {
  ASSERT_TRUE(LoadExtension(
      test_data_dir_.AppendASCII("good").AppendASCII("Extensions")
                    .AppendASCII("behllobkkfkfnphdnhnkndlbkcpglgmj")
                    .AppendASCII("1.0.0.0")));

  extensions::ProcessManager* manager =
      extensions::ProcessManager::Get(browser()->profile());
  extensions::ExtensionHost* host =
      FindHostWithPath(manager, "/backgroundpage.html", 1);
  ASSERT_TRUE(host->host_contents()->GetDelegate()->IsNeverVisible(
      host->host_contents()));
}

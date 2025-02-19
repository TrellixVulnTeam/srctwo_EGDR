// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/macros.h"
#include "chrome/browser/download/download_started_animation.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_commands.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/test/base/in_process_browser_test.h"

class DownloadStartedAnimationTest : public InProcessBrowserTest {
 public:
  DownloadStartedAnimationTest() {
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(DownloadStartedAnimationTest);
};

IN_PROC_BROWSER_TEST_F(DownloadStartedAnimationTest,
                       InstantiateAndImmediatelyClose) {
  content::WebContents* web_contents =
      browser()->tab_strip_model()->GetActiveWebContents();
  DownloadStartedAnimation::Show(web_contents);
  chrome::CloseWindow(browser());
}

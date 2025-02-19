// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/web_dialogs/test/test_web_contents_handler.h"

namespace ui {
namespace test {

TestWebContentsHandler::TestWebContentsHandler() {
}

TestWebContentsHandler::~TestWebContentsHandler() {
}

content::WebContents* TestWebContentsHandler::OpenURLFromTab(
      content::BrowserContext* context,
      content::WebContents* source,
      const content::OpenURLParams& params) {
  return NULL;
}

void TestWebContentsHandler::AddNewContents(content::BrowserContext* context,
                                            content::WebContents* source,
                                            content::WebContents* new_contents,
                                            WindowOpenDisposition disposition,
                                            const gfx::Rect& initial_rect,
                                            bool user_gesture) {
}

}  // namespace test
}  // namespace ui

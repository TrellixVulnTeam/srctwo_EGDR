// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/bookmarks/bookmark_editor_view.h"

#include "chrome/browser/ui/bookmarks/bookmark_utils_desktop.h"
#include "chrome/browser/ui/test/test_browser_dialog.h"

// Test harness for integration tests using BookmarkEditorView.
class BookmarkEditorViewBrowserTest : public DialogBrowserTest {
 public:
  BookmarkEditorViewBrowserTest() {}

  // DialogBrowserTest:
  void ShowDialog(const std::string& name) override {
    DCHECK_EQ("all_tabs", name);
    chrome::ShowBookmarkAllTabsDialog(browser());
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(BookmarkEditorViewBrowserTest);
};

// Shows the dialog for bookmarking all tabs. This shows a BookmarkEditorView
// dialog, with a tree view, where a user can rename and select a parent folder.
// Can be interactive when run with --gtest_filter=BrowserDialogTest.Invoke.
IN_PROC_BROWSER_TEST_F(BookmarkEditorViewBrowserTest, InvokeDialog_all_tabs) {
  RunDialog();
}

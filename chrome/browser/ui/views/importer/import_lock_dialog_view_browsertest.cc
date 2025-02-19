// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/importer/import_lock_dialog_view.h"

#include <string>

#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/test/test_browser_dialog.h"

class ImportLockDialogViewBrowserTest : public DialogBrowserTest {
 public:
  ImportLockDialogViewBrowserTest() {}

  // DialogBrowserTest:
  void ShowDialog(const std::string& name) override {
    gfx::NativeWindow native_window = browser()->window()->GetNativeWindow();
    ImportLockDialogView::Show(native_window, base::Callback<void(bool)>());
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ImportLockDialogViewBrowserTest);
};

// Invokes a dialog that implores the user to close Firefox before trying to
// import data. See test_browser_dialog.h.
IN_PROC_BROWSER_TEST_F(ImportLockDialogViewBrowserTest, InvokeDialog_default) {
  RunDialog();
}

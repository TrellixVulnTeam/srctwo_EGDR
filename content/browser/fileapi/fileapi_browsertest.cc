// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/path_service.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/test/content_browser_test.h"
#include "content/public/test/content_browser_test_utils.h"
#include "content/shell/browser/shell.h"
#include "content/test/content_browser_test_utils_internal.h"

namespace content {

// This browser test is aimed towards exercising the FileAPI bindings and
// the actual implementation that lives in the browser side.
class FileAPIBrowserTest : public ContentBrowserTest {
 public:
  FileAPIBrowserTest() {}
};

IN_PROC_BROWSER_TEST_F(FileAPIBrowserTest, FileInputChooserParams) {
  base::FilePath file;
  EXPECT_TRUE(PathService::Get(base::DIR_TEMP, &file));
  file = file.AppendASCII("bar");

  NavigateToURL(shell(), GetTestUrl(".", "file_input.html"));

  // Click on the <input type=file> element to launch the file upload picker.
  {
    std::unique_ptr<FileChooserDelegate> delegate(
        new FileChooserDelegate(file));
    shell()->web_contents()->SetDelegate(delegate.get());
    EXPECT_TRUE(ExecuteScript(shell(),
                              "document.getElementById('fileinput').click();"));
    EXPECT_TRUE(delegate->file_chosen());
    EXPECT_TRUE(delegate->params().default_file_name.empty());
  }

  // Click again, to verify what state was maintained and what was not.
  // The renderer is expected not to specify a default file name; it's up to
  // the browser to remember the last selected directory in the profile.
  {
    std::unique_ptr<FileChooserDelegate> delegate(
        new FileChooserDelegate(file));
    shell()->web_contents()->SetDelegate(delegate.get());
    EXPECT_TRUE(ExecuteScript(shell(),
                              "document.getElementById('fileinput').click();"));
    EXPECT_TRUE(delegate->file_chosen());
    EXPECT_TRUE(delegate->params().default_file_name.empty());
  }
}

}  // namespace content

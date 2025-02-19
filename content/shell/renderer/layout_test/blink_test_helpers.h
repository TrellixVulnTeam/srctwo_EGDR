// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_RENDERER_LAYOUT_TEST_BLINK_TEST_HELPERS_H_
#define CONTENT_SHELL_RENDERER_LAYOUT_TEST_BLINK_TEST_HELPERS_H_

namespace base {
class FilePath;
}

namespace test_runner {
struct TestPreferences;
}

namespace content {
struct WebPreferences;

// The TestRunner library keeps its settings in a TestPreferences object.
// The content_shell, however, uses WebPreferences. This method exports the
// settings from the TestRunner library which are relevant for layout tests.
void ExportLayoutTestSpecificPreferences(
    const test_runner::TestPreferences& from,
    WebPreferences* to);

// Applies settings that differ between layout tests and regular mode.
void ApplyLayoutTestDefaultPreferences(WebPreferences* prefs);

// Returns the root of the Blink checkout.
base::FilePath GetWebKitRootDirFilePath();

}  // namespace content

#endif  // CONTENT_SHELL_RENDERER_LAYOUT_TEST_BLINK_TEST_HELPERS_H_

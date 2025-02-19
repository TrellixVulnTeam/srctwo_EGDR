// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STORAGE_BROWSER_TEST_TEST_FILE_SYSTEM_OPTIONS_H_
#define STORAGE_BROWSER_TEST_TEST_FILE_SYSTEM_OPTIONS_H_

#include "storage/browser/fileapi/file_system_options.h"

namespace content {

// Returns Filesystem options for incognito mode.
storage::FileSystemOptions CreateIncognitoFileSystemOptions();

// Returns Filesystem options that allow file access.
storage::FileSystemOptions CreateAllowFileAccessOptions();

// Returns Filesystem options that disallow file access.
storage::FileSystemOptions CreateDisallowFileAccessOptions();

}  // namespace content

#endif  // STORAGE_BROWSER_TEST_TEST_FILE_SYSTEM_OPTIONS_H_

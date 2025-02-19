// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file only provides getStrings() as the .cc file for it is big.

#ifndef CHROME_BROWSER_CHROMEOS_EXTENSIONS_FILE_MANAGER_PRIVATE_API_STRINGS_H_
#define CHROME_BROWSER_CHROMEOS_EXTENSIONS_FILE_MANAGER_PRIVATE_API_STRINGS_H_

#include "chrome/browser/extensions/chrome_extension_function.h"

namespace extensions {

// Implements the chrome.fileManagerPrivate.getStrings method.
// Used to get strings for the file manager from JavaScript.
class FileManagerPrivateGetStringsFunction : public UIThreadExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("fileManagerPrivate.getStrings",
                             FILEMANAGERPRIVATE_GETSTRINGS)

  FileManagerPrivateGetStringsFunction();

 protected:
  ~FileManagerPrivateGetStringsFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;
};

}  // namespace extensions

#endif  // CHROME_BROWSER_CHROMEOS_EXTENSIONS_FILE_MANAGER_PRIVATE_API_STRINGS_H_

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/base/platform_mime_util.h"

#include <string>

#include "base/strings/utf_string_conversions.h"
#include "base/win/registry.h"

namespace net {

bool PlatformMimeUtil::GetPlatformMimeTypeFromExtension(
    const base::FilePath::StringType& ext, std::string* result) const {
  // check windows registry for file extension's mime type (registry key
  // names are not case-sensitive).
  std::wstring value, key = L"." + ext;
  base::win::RegKey(HKEY_CLASSES_ROOT, key.c_str(), KEY_READ).ReadValue(
      L"Content Type", &value);
  if (!value.empty()) {
    *result = base::WideToUTF8(value);
    return true;
  }
  return false;
}

bool PlatformMimeUtil::GetPlatformPreferredExtensionForMimeType(
    const std::string& mime_type,
    base::FilePath::StringType* ext) const {
  std::wstring key(
      L"MIME\\Database\\Content Type\\" + base::UTF8ToWide(mime_type));
  if (base::win::RegKey(HKEY_CLASSES_ROOT, key.c_str(), KEY_READ).ReadValue(
          L"Extension", ext) != ERROR_SUCCESS) {
    return false;
  }
  // Strip off the leading dot, this should always be the case.
  if (!ext->empty() && ext->at(0) == L'.')
    ext->erase(ext->begin());

  return true;
}

void PlatformMimeUtil::GetPlatformExtensionsForMimeType(
    const std::string& mime_type,
    std::unordered_set<base::FilePath::StringType>* extensions) const {
  // Multiple extensions could have the given mime type specified as their types
  // in their 'HKCR\.<extension>\Content Type' keys. Iterating all the HKCR
  // entries, though, is wildly impractical. Cheat by returning just the
  // preferred extension.
  base::FilePath::StringType ext;
  if (GetPlatformPreferredExtensionForMimeType(mime_type, &ext))
    extensions->insert(ext);
}

}  // namespace net

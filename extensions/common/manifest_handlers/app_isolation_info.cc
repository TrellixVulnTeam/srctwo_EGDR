// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/common/manifest_handlers/app_isolation_info.h"

#include <stddef.h>

#include <memory>

#include "base/memory/ptr_util.h"
#include "base/strings/string16.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "extensions/common/error_utils.h"
#include "extensions/common/manifest_constants.h"
#include "extensions/common/manifest_handlers/permissions_parser.h"
#include "extensions/common/permissions/api_permission_set.h"

namespace extensions {

namespace keys = manifest_keys;

AppIsolationInfo::AppIsolationInfo(bool isolated_storage)
    : has_isolated_storage(isolated_storage) {
}

AppIsolationInfo::~AppIsolationInfo() {
}

// static
bool AppIsolationInfo::HasIsolatedStorage(const Extension* extension) {
  AppIsolationInfo* info = static_cast<AppIsolationInfo*>(
      extension->GetManifestData(keys::kIsolation));
  return info ? info->has_isolated_storage : false;
}

AppIsolationHandler::AppIsolationHandler() {
}

AppIsolationHandler::~AppIsolationHandler() {
}

bool AppIsolationHandler::Parse(Extension* extension, base::string16* error) {
  // Platform apps always get isolated storage.
  if (extension->is_platform_app()) {
    extension->SetManifestData(keys::kIsolation,
                               std::make_unique<AppIsolationInfo>(true));
    return true;
  }

  // Other apps only get it if it is requested _and_ experimental APIs are
  // enabled.
  if (!extension->is_app() ||
      !PermissionsParser::HasAPIPermission(extension,
                                           APIPermission::kExperimental)) {
    return true;
  }

  // We should only be parsing if the extension has the key in the manifest,
  // or is a platform app (which we already handled).
  DCHECK(extension->manifest()->HasPath(keys::kIsolation));

  const base::ListValue* isolation_list = NULL;
  if (!extension->manifest()->GetList(keys::kIsolation, &isolation_list)) {
    *error = base::ASCIIToUTF16(manifest_errors::kInvalidIsolation);
    return false;
  }

  bool has_isolated_storage = false;
  for (size_t i = 0; i < isolation_list->GetSize(); ++i) {
    std::string isolation_string;
    if (!isolation_list->GetString(i, &isolation_string)) {
      *error = ErrorUtils::FormatErrorMessageUTF16(
          manifest_errors::kInvalidIsolationValue, base::SizeTToString(i));
      return false;
    }

    // Check for isolated storage.
    if (isolation_string == manifest_values::kIsolatedStorage) {
      has_isolated_storage = true;
    } else {
      DLOG(WARNING) << "Did not recognize isolation type: " << isolation_string;
    }
  }

  if (has_isolated_storage)
    extension->SetManifestData(keys::kIsolation,
                               std::make_unique<AppIsolationInfo>(true));

  return true;
}

bool AppIsolationHandler::AlwaysParseForType(Manifest::Type type) const {
  return type == Manifest::TYPE_PLATFORM_APP;
}

const std::vector<std::string> AppIsolationHandler::Keys() const {
  return SingleKey(keys::kIsolation);
}

}  // namespace extensions

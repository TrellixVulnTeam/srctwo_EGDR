// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/common/manifest_handlers/icons_handler.h"

#include <memory>

#include "base/files/file_util.h"
#include "base/lazy_instance.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"
#include "extensions/common/file_util.h"
#include "extensions/common/manifest_constants.h"
#include "extensions/common/manifest_handler_helpers.h"
#include "extensions/strings/grit/extensions_strings.h"
#include "ui/gfx/geometry/size.h"

namespace extensions {

namespace keys = manifest_keys;

static base::LazyInstance<ExtensionIconSet>::DestructorAtExit g_empty_icon_set =
    LAZY_INSTANCE_INITIALIZER;

// static
const ExtensionIconSet& IconsInfo::GetIcons(const Extension* extension) {
  DCHECK(extension);
  IconsInfo* info = static_cast<IconsInfo*>(
      extension->GetManifestData(keys::kIcons));
  return info ? info->icons : g_empty_icon_set.Get();
}

// static
ExtensionResource IconsInfo::GetIconResource(
    const Extension* extension,
    int size,
    ExtensionIconSet::MatchType match_type) {
  const std::string& path = GetIcons(extension).Get(size, match_type);
  return path.empty() ? ExtensionResource() : extension->GetResource(path);
}

// static
GURL IconsInfo::GetIconURL(const Extension* extension,
                           int size,
                           ExtensionIconSet::MatchType match_type) {
  const std::string& path = GetIcons(extension).Get(size, match_type);
  return path.empty() ? GURL() : extension->GetResourceURL(path);
}

IconsHandler::IconsHandler() {
}

IconsHandler::~IconsHandler() {
}

bool IconsHandler::Parse(Extension* extension, base::string16* error) {
  std::unique_ptr<IconsInfo> icons_info(new IconsInfo);
  const base::DictionaryValue* icons_dict = NULL;
  if (!extension->manifest()->GetDictionary(keys::kIcons, &icons_dict)) {
    *error = base::ASCIIToUTF16(manifest_errors::kInvalidIcons);
    return false;
  }

  if (!manifest_handler_helpers::LoadIconsFromDictionary(
          icons_dict, &icons_info->icons, error)) {
    return false;
  }

  extension->SetManifestData(keys::kIcons, std::move(icons_info));
  return true;
}

bool IconsHandler::Validate(const Extension* extension,
                            std::string* error,
                            std::vector<InstallWarning>* warnings) const {
  return file_util::ValidateExtensionIconSet(IconsInfo::GetIcons(extension),
                                             extension,
                                             IDS_EXTENSION_LOAD_ICON_FAILED,
                                             error);
}

const std::vector<std::string> IconsHandler::Keys() const {
  return SingleKey(keys::kIcons);
}

}  // namespace extensions

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_COMMON_MANIFEST_HANDLERS_OFFLINE_ENABLED_INFO_H_
#define EXTENSIONS_COMMON_MANIFEST_HANDLERS_OFFLINE_ENABLED_INFO_H_

#include <string>
#include <vector>

#include "base/macros.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest.h"
#include "extensions/common/manifest_handler.h"

namespace extensions {

struct OfflineEnabledInfo : public Extension::ManifestData {
  explicit OfflineEnabledInfo(bool offline_enabled);
  ~OfflineEnabledInfo() override;

  // Whether the extension or app should be enabled when offline.
  // Defaults to false, except for platform apps which are offline by default.
  bool offline_enabled;

  static bool IsOfflineEnabled(const Extension* extension);
};

// Parses the "offline_enabled" manifest key.
class OfflineEnabledHandler : public ManifestHandler {
 public:
  OfflineEnabledHandler();
  ~OfflineEnabledHandler() override;

  bool Parse(Extension* extension, base::string16* error) override;
  bool AlwaysParseForType(Manifest::Type type) const override;

 private:
  const std::vector<std::string> Keys() const override;

  DISALLOW_COPY_AND_ASSIGN(OfflineEnabledHandler);
};

}  // namespace extensions

#endif  // EXTENSIONS_COMMON_MANIFEST_HANDLERS_OFFLINE_ENABLED_INFO_H_

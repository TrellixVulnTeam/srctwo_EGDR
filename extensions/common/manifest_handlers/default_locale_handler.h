// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_COMMON_MANIFEST_HANDLERS_DEFAULT_LOCALE_HANDLER_H_
#define EXTENSIONS_COMMON_MANIFEST_HANDLERS_DEFAULT_LOCALE_HANDLER_H_

#include <string>

#include "base/macros.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest_handler.h"

namespace extensions {

// A structure to hold the locale information for an extension.
struct LocaleInfo : public Extension::ManifestData {
  // Default locale for fall back. Can be empty if extension is not localized.
  std::string default_locale;

  static const std::string& GetDefaultLocale(const Extension* extension);
};

// Parses the "default_locale" manifest key.
class DefaultLocaleHandler : public ManifestHandler {
 public:
  DefaultLocaleHandler();
  ~DefaultLocaleHandler() override;

  bool Parse(Extension* extension, base::string16* error) override;

  // Validates locale info. Doesn't check if messages.json files are valid.
  bool Validate(const Extension* extension,
                std::string* error,
                std::vector<InstallWarning>* warnings) const override;

  bool AlwaysValidateForType(Manifest::Type type) const override;

 private:
  const std::vector<std::string> Keys() const override;

  DISALLOW_COPY_AND_ASSIGN(DefaultLocaleHandler);
};

}  // namespace extensions

#endif  // EXTENSIONS_COMMON_MANIFEST_HANDLERS_DEFAULT_LOCALE_HANDLER_H_

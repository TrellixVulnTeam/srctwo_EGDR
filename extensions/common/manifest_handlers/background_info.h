// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_COMMON_MANIFEST_HANDLERS_BACKGROUND_INFO_H_
#define EXTENSIONS_COMMON_MANIFEST_HANDLERS_BACKGROUND_INFO_H_

#include <string>
#include <vector>

#include "base/macros.h"
#include "base/values.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest_handler.h"
#include "url/gurl.h"

namespace extensions {

class BackgroundInfo : public Extension::ManifestData {
 public:
  BackgroundInfo();
  ~BackgroundInfo() override;

  static GURL GetBackgroundURL(const Extension* extension);
  static const std::vector<std::string>& GetBackgroundScripts(
      const Extension* extension);
  static bool HasBackgroundPage(const Extension* extension);
  static bool HasPersistentBackgroundPage(const Extension* extension);
  static bool HasLazyBackgroundPage(const Extension* extension);
  static bool HasGeneratedBackgroundPage(const Extension* extension);
  static bool AllowJSAccess(const Extension* extension);

  bool has_background_page() const {
    return background_url_.is_valid() || !background_scripts_.empty();
  }

  bool has_persistent_background_page() const {
    return has_background_page() && is_persistent_;
  }

  bool has_lazy_background_page() const {
    return has_background_page() && !is_persistent_;
  }

  bool Parse(const Extension* extension, base::string16* error);

 private:
  bool LoadBackgroundScripts(const Extension* extension,
                             const std::string& key,
                             base::string16* error);
  bool LoadBackgroundPage(const Extension* extension,
                          const std::string& key,
                          base::string16* error);
  bool LoadBackgroundPage(const Extension* extension, base::string16* error);
  bool LoadBackgroundPersistent(const Extension* extension,
                                base::string16* error);
  bool LoadAllowJSAccess(const Extension* extension, base::string16* error);

  // Optional URL to a master page of which a single instance should be always
  // loaded in the background.
  GURL background_url_;

  // Optional list of scripts to use to generate a background page. If this is
  // present, background_url_ will be empty and generated by GetBackgroundURL().
  std::vector<std::string> background_scripts_;

  // True if the background page should stay loaded forever; false if it should
  // load on-demand (when it needs to handle an event). Defaults to true.
  bool is_persistent_;

  // True if the background page can be scripted by pages of the app or
  // extension, in which case all such pages must run in the same process.
  // False if such pages are not permitted to script the background page,
  // allowing them to run in different processes.
  // Defaults to true.
  bool allow_js_access_;

  DISALLOW_COPY_AND_ASSIGN(BackgroundInfo);
};

// Parses all background/event page-related keys in the manifest.
class BackgroundManifestHandler : public ManifestHandler {
 public:
  BackgroundManifestHandler();
  ~BackgroundManifestHandler() override;

  bool Parse(Extension* extension, base::string16* error) override;
  bool Validate(const Extension* extension,
                std::string* error,
                std::vector<InstallWarning>* warnings) const override;
  bool AlwaysParseForType(Manifest::Type type) const override;

 private:
  const std::vector<std::string> Keys() const override;

  DISALLOW_COPY_AND_ASSIGN(BackgroundManifestHandler);
};

}  // namespace extensions

#endif  // EXTENSIONS_COMMON_MANIFEST_HANDLERS_BACKGROUND_INFO_H_

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_COMMON_EXTENSIONS_MANIFEST_HANDLERS_APP_LAUNCH_INFO_H_
#define CHROME_COMMON_EXTENSIONS_MANIFEST_HANDLERS_APP_LAUNCH_INFO_H_

#include <string>
#include <vector>

#include "base/macros.h"
#include "chrome/common/extensions/extension_constants.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest.h"
#include "extensions/common/manifest_handler.h"
#include "url/gurl.h"

namespace extensions {

// Container that holds the parsed app launch data.
class AppLaunchInfo : public Extension::ManifestData {
 public:
  AppLaunchInfo();
  ~AppLaunchInfo() override;

  // Get the local path inside the extension to use with the launcher.
  static const std::string& GetLaunchLocalPath(const Extension* extension);

  // Get the absolute web url to use with the launcher.
  static const GURL& GetLaunchWebURL(const Extension* extension);

  // The window type that an app's manifest specifies to launch into.
  // This is not always the window type an app will open into, because
  // users can override the way each app launches.  See
  // ExtensionPrefs::GetLaunchContainer(), which looks at a per-app pref
  // to decide what container an app will launch in.
  static LaunchContainer GetLaunchContainer(
      const Extension* extension);

  // The default size of the container when launching. Only respected for
  // containers like panels and windows.
  static int GetLaunchWidth(const Extension* extension);
  static int GetLaunchHeight(const Extension* extension);

  // Get the fully resolved absolute launch URL.
  static GURL GetFullLaunchURL(const Extension* extension);

  bool Parse(Extension* extension, base::string16* error);

 private:
  bool LoadLaunchURL(Extension* extension, base::string16* error);
  bool LoadLaunchContainer(Extension* extension, base::string16* error);
  void OverrideLaunchURL(Extension* extension, GURL override_url);

  std::string launch_local_path_;

  GURL launch_web_url_;

  LaunchContainer launch_container_;

  int launch_width_;
  int launch_height_;

  DISALLOW_COPY_AND_ASSIGN(AppLaunchInfo);
};

// Parses all app launch related keys in the manifest.
class AppLaunchManifestHandler : public ManifestHandler {
 public:
  AppLaunchManifestHandler();
  ~AppLaunchManifestHandler() override;

  bool Parse(Extension* extension, base::string16* error) override;
  bool AlwaysParseForType(Manifest::Type type) const override;

 private:
  const std::vector<std::string> Keys() const override;

  DISALLOW_COPY_AND_ASSIGN(AppLaunchManifestHandler);
};

}  // namespace extensions

#endif  // CHROME_COMMON_EXTENSIONS_MANIFEST_HANDLERS_APP_LAUNCH_INFO_H_

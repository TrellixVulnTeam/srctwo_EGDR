// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_CHROME_TEST_EXTENSION_LOADER_H_
#define CHROME_BROWSER_EXTENSIONS_CHROME_TEST_EXTENSION_LOADER_H_

#include <string>

#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/macros.h"
#include "base/optional.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest.h"

namespace base {
class FilePath;
}

namespace content {
class BrowserContext;
}

class ExtensionService;

namespace extensions {
class ExtensionRegistry;
class ExtensionSystem;

// A test class to help with loading packed or unpacked extensions. Designed to
// be used by both browser tests and unit tests. Note that this should be used
// for a single extension, and is designed to be used on the stack (rather than
// as a test suite member).
class ChromeTestExtensionLoader {
 public:
  explicit ChromeTestExtensionLoader(content::BrowserContext* browser_context);
  ~ChromeTestExtensionLoader();

  // Loads the extension specified by |file_path|. Works for both packed and
  // unpacked extensions.
  scoped_refptr<const Extension> LoadExtension(const base::FilePath& file_path);

  // Myriad different settings. See the member variable declarations for
  // explanations and defaults.
  // Prefer using these setters rather than adding n different
  // LoadExtensionWith* variants (that's not scalable).
  void set_expected_id(const std::string& expected_id) {
    expected_id_ = expected_id;
  }
  void add_creation_flag(Extension::InitFromValueFlags flag) {
    creation_flags_ |= flag;
  }
  void set_creation_flags(int flags) { creation_flags_ = flags; }
  void set_location(Manifest::Location location) { location_ = location; }
  void set_should_fail(bool should_fail) { should_fail_ = should_fail; }
  void set_pack_extension(bool pack_extension) {
    pack_extension_ = pack_extension;
  }
  void set_install_immediately(bool install_immediately) {
    install_immediately_ = install_immediately;
  }
  void set_grant_permissions(bool grant_permissions) {
    grant_permissions_ = grant_permissions;
  }
  void set_allow_file_access(bool allow_file_access) {
    allow_file_access_ = allow_file_access;
  }
  void set_allow_incognito_access(bool allow_incognito_access) {
    allow_incognito_access_ = allow_incognito_access;
  }
  void set_ignore_manifest_warnings(bool ignore_manifest_warnings) {
    ignore_manifest_warnings_ = ignore_manifest_warnings;
  }
  void set_require_modern_manifest_version(bool require_modern_version) {
    require_modern_manifest_version_ = require_modern_version;
  }
  void set_install_param(const std::string& install_param) {
    install_param_ = install_param;
  }

 private:
  // Packs the extension at |unpacked_path| and returns the path to the created
  // crx. Note that the created crx is tied to the lifetime of |this|.
  base::FilePath PackExtension(const base::FilePath& unpacked_path);

  // Loads the crx pointed to by |crx_path|.
  scoped_refptr<const Extension> LoadCrx(const base::FilePath& crx_path);

  // Loads the unpacked extension pointed to by |unpacked_path|.
  scoped_refptr<const Extension> LoadUnpacked(
      const base::FilePath& unpacked_path);

  // Checks that the permissions of the loaded extension are correct.
  void CheckPermissions(const Extension* extension);

  // Checks for any install warnings associated with the extension.
  bool CheckInstallWarnings(const Extension& extension);

  // Waits for the extension to finish setting up.
  bool WaitForExtensionReady();

  // The associated context and services.
  content::BrowserContext* browser_context_ = nullptr;
  ExtensionSystem* extension_system_ = nullptr;
  ExtensionService* extension_service_ = nullptr;
  ExtensionRegistry* extension_registry_ = nullptr;

  // A temporary directory for packing extensions.
  base::ScopedTempDir temp_dir_;

  // The extension id of the loaded extension.
  std::string extension_id_;

  // A provided PEM path to use. If not provided, a temporary one will be
  // created.
  base::FilePath pem_path_;

  // The expected extension id, if any.
  std::string expected_id_;

  // An install param to use with the loaded extension.
  std::string install_param_;

  // Any creation flags (see Extension::InitFromValueFlags) to use for the
  // extension. Only used for crx installs.
  int creation_flags_ = Extension::NO_FLAGS;

  // The install location of the added extension. Not valid for unpacked
  // extensions.
  Manifest::Location location_ = Manifest::INTERNAL;

  // Whether or not the extension load should fail.
  bool should_fail_ = false;

  // Whether or not to always pack the extension before loading it. Otherwise,
  // the extension will be loaded as an unpacked extension.
  bool pack_extension_ = false;

  // Whether or not to install the extension immediately. Only used for crx
  // installs.
  bool install_immediately_ = true;

  // Whether or not to automatically grant permissions to the installed
  // extension. Only used for crx installs.
  bool grant_permissions_ = true;

  // Whether or not to allow file access by default to the extension.
  base::Optional<bool> allow_file_access_;

  // Whether or not to allow incognito access by default to the extension.
  bool allow_incognito_access_ = false;

  // Whether or not to ignore manifest warnings during installation.
  bool ignore_manifest_warnings_ = false;

  // Whether or not to enforce a minimum manifest version requirement.
  bool require_modern_manifest_version_ = true;

  DISALLOW_COPY_AND_ASSIGN(ChromeTestExtensionLoader);
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_CHROME_TEST_EXTENSION_LOADER_H_

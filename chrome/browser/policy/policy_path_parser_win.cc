// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/policy/policy_path_parser.h"

#include <shlobj.h>
#include <stddef.h>
#include <wtsapi32.h>

#include <memory>

#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/registry.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/install_static/policy_path_parser.h"
#include "components/policy/policy_constants.h"

namespace {

// Checks if the key exists in the given hive and expands any string variables.
bool LoadUserDataDirPolicyFromRegistry(HKEY hive,
                                       const char* key_name_str,
                                       base::FilePath* dir) {
  base::string16 value;
  base::string16 key_name(base::ASCIIToUTF16(key_name_str));
  base::win::RegKey key(hive, policy::kRegistryChromePolicyKey, KEY_READ);
  if (key.ReadValue(key_name.c_str(), &value) == ERROR_SUCCESS) {
    *dir = base::FilePath(policy::path_parser::ExpandPathVariables(value));
    return true;
  }
  return false;
}

}  // namespace

namespace policy {

namespace path_parser {

// Replaces all variable occurances in the policy string with the respective
// system settings values.
base::FilePath::StringType ExpandPathVariables(
    const base::FilePath::StringType& untranslated_string) {
  // This is implemented in the install_static library so that it can also be
  // used by Crashpad initialization code in chrome_elf, which has a very
  // constrained set of dependencies.
  return install_static::ExpandPathVariables(untranslated_string);
}

void CheckUserDataDirPolicy(base::FilePath* user_data_dir) {
  DCHECK(user_data_dir);
  // Policy from the HKLM hive has precedence over HKCU.
  if (!LoadUserDataDirPolicyFromRegistry(HKEY_LOCAL_MACHINE, key::kUserDataDir,
                                         user_data_dir)) {
    LoadUserDataDirPolicyFromRegistry(HKEY_CURRENT_USER, key::kUserDataDir,
                                      user_data_dir);
  }
}

void CheckDiskCacheDirPolicy(base::FilePath* disk_cache_dir) {
  DCHECK(disk_cache_dir);
  if (!LoadUserDataDirPolicyFromRegistry(HKEY_LOCAL_MACHINE, key::kDiskCacheDir,
                                         disk_cache_dir)) {
    LoadUserDataDirPolicyFromRegistry(HKEY_CURRENT_USER, key::kDiskCacheDir,
                                      disk_cache_dir);
  }
}

}  // namespace path_parser

}  // namespace policy

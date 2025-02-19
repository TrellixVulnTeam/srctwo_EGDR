// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/common/importer/importer_test_registry_overrider_win.h"

#include <windows.h>

#include <memory>
#include <string>

#include "base/environment.h"
#include "base/guid.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/registry.h"

namespace {

// The key to which a random subkey will be appended. This key itself will never
// be deleted.
const wchar_t kTestHKCUOverrideKeyPrefix[] = L"SOFTWARE\\Chromium Unit Tests\\";
const char kTestHKCUOverrideEnvironmentVariable[] =
    "IE_IMPORTER_TEST_OVERRIDE_HKCU";

// Reads the environment variable set by a previous call to
// SetTestRegistryOverride() into |key| if it exists and |key| is not NULL.
// Returns true if the variable was successfully read.
bool GetTestKeyFromEnvironment(base::string16* key) {
  std::unique_ptr<base::Environment> env(base::Environment::Create());
  std::string value;
  bool result = env->GetVar(kTestHKCUOverrideEnvironmentVariable, &value);
  if (result)
    *key = base::UTF8ToUTF16(value);
  return result;
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// ImporterTestRegistryOverrider, public:

ImporterTestRegistryOverrider::ImporterTestRegistryOverrider()
    : temporary_key_(kTestHKCUOverrideKeyPrefix +
                     base::UTF8ToUTF16(base::GenerateGUID())) {
  DCHECK(!GetTestKeyFromEnvironment(NULL));

  std::unique_ptr<base::Environment> env(base::Environment::Create());
  bool success = env->SetVar(kTestHKCUOverrideEnvironmentVariable,
                             base::UTF16ToUTF8(temporary_key_));
  DCHECK(success);
}

ImporterTestRegistryOverrider::~ImporterTestRegistryOverrider() {
  base::win::RegKey reg_key(HKEY_CURRENT_USER, temporary_key_.c_str(),
                            KEY_ALL_ACCESS);
  DCHECK(reg_key.Valid());
  reg_key.DeleteKey(L"");

  std::unique_ptr<base::Environment> env(base::Environment::Create());
  bool success = env->UnSetVar(kTestHKCUOverrideEnvironmentVariable);
  DCHECK(success);
}

// static
base::string16 ImporterTestRegistryOverrider::GetTestRegistryOverride() {
  base::string16 key;
  if (!GetTestKeyFromEnvironment(&key))
    return base::string16();
  return key;
}

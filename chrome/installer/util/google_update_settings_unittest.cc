// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/installer/util/google_update_settings.h"

#include <windows.h>
#include <shlwapi.h>  // For SHDeleteKey.
#include <stddef.h>

#include <memory>

#include "base/base_paths.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "base/test/scoped_path_override.h"
#include "base/test/test_reg_util_win.h"
#include "base/win/registry.h"
#include "base/win/win_util.h"
#include "chrome/common/chrome_constants.h"
#include "chrome/install_static/install_util.h"
#include "chrome/install_static/test/scoped_install_details.h"
#include "chrome/installer/util/app_registration_data.h"
#include "chrome/installer/util/browser_distribution.h"
#include "chrome/installer/util/channel_info.h"
#include "chrome/installer/util/fake_installation_state.h"
#include "chrome/installer/util/google_update_constants.h"
#include "chrome/installer/util/helper.h"
#include "chrome/installer/util/util_constants.h"
#include "chrome/installer/util/work_item_list.h"
#include "testing/gtest/include/gtest/gtest.h"

using base::win::RegKey;
using installer::ChannelInfo;

namespace {

const wchar_t kTestProductGuid[] = L"{89F1B351-B15D-48D4-8F10-1298721CF13D}";

const wchar_t kTestExperimentLabel[] = L"test_label_value";

// This test fixture redirects the HKLM and HKCU registry hives for
// the duration of the test to make it independent of the machine
// and user settings.
class GoogleUpdateSettingsTest : public testing::Test {
 protected:
  enum SystemUserInstall {
    SYSTEM_INSTALL,
    USER_INSTALL,
  };

  GoogleUpdateSettingsTest()
      : program_files_override_(base::DIR_PROGRAM_FILES),
        program_files_x86_override_(base::DIR_PROGRAM_FILESX86) {}

  void SetUp() override {
    ASSERT_NO_FATAL_FAILURE(
        registry_overrides_.OverrideRegistry(HKEY_LOCAL_MACHINE));
    ASSERT_NO_FATAL_FAILURE(
        registry_overrides_.OverrideRegistry(HKEY_CURRENT_USER));
  }

  // Test the writing and deleting functionality of the experiments label
  // helper.
  void TestExperimentsLabelHelper(SystemUserInstall install) {
    // Install a basic InstallDetails instance.
    install_static::ScopedInstallDetails details(install == SYSTEM_INSTALL);

    base::string16 value;
    // Before anything is set, ReadExperimentLabels should succeed but return
    // an empty string.
    EXPECT_TRUE(GoogleUpdateSettings::ReadExperimentLabels(
        install == SYSTEM_INSTALL, &value));
    EXPECT_EQ(base::string16(), value);

    EXPECT_TRUE(GoogleUpdateSettings::SetExperimentLabels(
        install == SYSTEM_INSTALL, kTestExperimentLabel));

    // Validate that something is written. Only worry about the label itself.
    RegKey key;
    HKEY root = install == SYSTEM_INSTALL ?
        HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
    BrowserDistribution* chrome = BrowserDistribution::GetDistribution();
    base::string16 state_key = install == SYSTEM_INSTALL ?
        chrome->GetStateMediumKey() : chrome->GetStateKey();

    EXPECT_EQ(ERROR_SUCCESS,
              key.Open(root, state_key.c_str(), KEY_QUERY_VALUE));
    EXPECT_EQ(ERROR_SUCCESS,
        key.ReadValue(google_update::kExperimentLabels, &value));
    EXPECT_EQ(kTestExperimentLabel, value);
    EXPECT_TRUE(GoogleUpdateSettings::ReadExperimentLabels(
        install == SYSTEM_INSTALL, &value));
    EXPECT_EQ(kTestExperimentLabel, value);
    key.Close();

    // Now that the label is set, test the delete functionality. An empty label
    // should result in deleting the value.
    EXPECT_TRUE(GoogleUpdateSettings::SetExperimentLabels(
        install == SYSTEM_INSTALL, base::string16()));
    EXPECT_EQ(ERROR_SUCCESS,
              key.Open(root, state_key.c_str(), KEY_QUERY_VALUE));
    EXPECT_EQ(ERROR_FILE_NOT_FOUND,
        key.ReadValue(google_update::kExperimentLabels, &value));
    EXPECT_TRUE(GoogleUpdateSettings::ReadExperimentLabels(
        install == SYSTEM_INSTALL, &value));
    EXPECT_EQ(base::string16(), value);
    key.Close();
  }

  // Creates "ap" key with the value given as parameter. Also adds work
  // items to work_item_list given so that they can be rolled back later.
  bool CreateApKey(WorkItemList* work_item_list, const base::string16& value) {
    HKEY reg_root = HKEY_CURRENT_USER;
    base::string16 reg_key = GetApKeyPath();
    work_item_list->AddCreateRegKeyWorkItem(
        reg_root, reg_key, WorkItem::kWow64Default);
    work_item_list->AddSetRegValueWorkItem(reg_root,
                                           reg_key,
                                           WorkItem::kWow64Default,
                                           google_update::kRegApField,
                                           value.c_str(),
                                           true);
    if (!work_item_list->Do()) {
      work_item_list->Rollback();
      return false;
    }
    return true;
  }

  // Returns the key path of "ap" key, e.g.:
  // Google\Update\ClientState\<kTestProductGuid>
  base::string16 GetApKeyPath() {
    base::string16 reg_key(google_update::kRegPathClientState);
    reg_key.append(L"\\");
    reg_key.append(kTestProductGuid);
    return reg_key;
  }

  // Utility method to read "ap" key value
  base::string16 ReadApKeyValue() {
    RegKey key;
    base::string16 ap_key_value;
    base::string16 reg_key = GetApKeyPath();
    if (key.Open(HKEY_CURRENT_USER, reg_key.c_str(), KEY_ALL_ACCESS) ==
        ERROR_SUCCESS) {
      key.ReadValue(google_update::kRegApField, &ap_key_value);
    }

    return ap_key_value;
  }

  bool SetUpdatePolicyForAppGuid(const base::string16& app_guid,
                                 GoogleUpdateSettings::UpdatePolicy policy) {
    RegKey policy_key;
    if (policy_key.Create(HKEY_LOCAL_MACHINE,
                          GoogleUpdateSettings::kPoliciesKey,
                          KEY_SET_VALUE) == ERROR_SUCCESS) {
      base::string16 app_update_override(
          GoogleUpdateSettings::kUpdateOverrideValuePrefix);
      app_update_override.append(app_guid);
      return policy_key.WriteValue(app_update_override.c_str(),
                                   static_cast<DWORD>(policy)) == ERROR_SUCCESS;
    }
    return false;
  }

  GoogleUpdateSettings::UpdatePolicy GetUpdatePolicyForAppGuid(
      const base::string16& app_guid) {
    RegKey policy_key;
    if (policy_key.Create(HKEY_LOCAL_MACHINE,
                          GoogleUpdateSettings::kPoliciesKey,
                          KEY_QUERY_VALUE) == ERROR_SUCCESS) {
      base::string16 app_update_override(
          GoogleUpdateSettings::kUpdateOverrideValuePrefix);
      app_update_override.append(app_guid);

      DWORD value;
      if (policy_key.ReadValueDW(app_update_override.c_str(),
                                 &value) == ERROR_SUCCESS) {
        return static_cast<GoogleUpdateSettings::UpdatePolicy>(value);
      }
    }
    return GoogleUpdateSettings::UPDATE_POLICIES_COUNT;
  }

  bool SetGlobalUpdatePolicy(GoogleUpdateSettings::UpdatePolicy policy) {
    RegKey policy_key;
    return policy_key.Create(HKEY_LOCAL_MACHINE,
                             GoogleUpdateSettings::kPoliciesKey,
                             KEY_SET_VALUE) == ERROR_SUCCESS &&
           policy_key.WriteValue(GoogleUpdateSettings::kUpdatePolicyValue,
                                 static_cast<DWORD>(policy)) == ERROR_SUCCESS;
  }

  GoogleUpdateSettings::UpdatePolicy GetGlobalUpdatePolicy() {
    RegKey policy_key;
    DWORD value;
    return (policy_key.Create(HKEY_LOCAL_MACHINE,
                              GoogleUpdateSettings::kPoliciesKey,
                              KEY_QUERY_VALUE) == ERROR_SUCCESS &&
            policy_key.ReadValueDW(GoogleUpdateSettings::kUpdatePolicyValue,
                                   &value) == ERROR_SUCCESS) ?
        static_cast<GoogleUpdateSettings::UpdatePolicy>(value) :
        GoogleUpdateSettings::UPDATE_POLICIES_COUNT;
  }

  bool SetUpdateTimeoutOverride(DWORD time_in_minutes) {
    RegKey policy_key;
    return policy_key.Create(HKEY_LOCAL_MACHINE,
                             GoogleUpdateSettings::kPoliciesKey,
                             KEY_SET_VALUE) == ERROR_SUCCESS &&
           policy_key.WriteValue(
               GoogleUpdateSettings::kCheckPeriodOverrideMinutes,
               time_in_minutes) == ERROR_SUCCESS;
  }

  // Path overrides so that SHGetFolderPath isn't needed after the registry
  // is overridden.
  base::ScopedPathOverride program_files_override_;
  base::ScopedPathOverride program_files_x86_override_;
  registry_util::RegistryOverrideManager registry_overrides_;
};

}  // namespace

// Run through all combinations of diff vs. full install, success and failure
// results, and a fistful of initial "ap" values checking that the expected
// final "ap" value is generated by
// GoogleUpdateSettings::UpdateGoogleUpdateApKey.
TEST_F(GoogleUpdateSettingsTest, UpdateGoogleUpdateApKey) {
  const installer::ArchiveType archive_types[] = {
    installer::UNKNOWN_ARCHIVE_TYPE,
    installer::FULL_ARCHIVE_TYPE,
    installer::INCREMENTAL_ARCHIVE_TYPE
  };
  const int results[] = {
    installer::FIRST_INSTALL_SUCCESS,
    installer::INSTALL_FAILED
  };
  const wchar_t* const plain[] = {
    L"",
    L"1.1",
    L"1.1-dev"
  };
  const wchar_t* const full[] = {
    L"-full",
    L"1.1-full",
    L"1.1-dev-full"
  };
  static_assert(arraysize(full) == arraysize(plain), "bad full array size");
  const wchar_t* const multifail[] = {
    L"-multifail",
    L"1.1-multifail",
    L"1.1-dev-multifail"
  };
  static_assert(arraysize(multifail) == arraysize(plain),
                "bad multifail array size");
  const wchar_t* const multifail_full[] = {
    L"-multifail-full",
    L"1.1-multifail-full",
    L"1.1-dev-multifail-full"
  };
  static_assert(arraysize(multifail_full) == arraysize(plain),
                "bad multifail_full array size");
  const wchar_t* const* input_arrays[] = {
    plain,
    full,
    multifail,
    multifail_full
  };
  ChannelInfo v;
  for (const installer::ArchiveType archive_type : archive_types) {
    for (const int result : results) {
      // The archive type will/must always be known on install success.
      if (archive_type == installer::UNKNOWN_ARCHIVE_TYPE &&
          result == installer::FIRST_INSTALL_SUCCESS) {
        continue;
      }
      const wchar_t* const* outputs = NULL;
      if (result == installer::FIRST_INSTALL_SUCCESS ||
          archive_type == installer::FULL_ARCHIVE_TYPE) {
        outputs = plain;
      } else if (archive_type == installer::INCREMENTAL_ARCHIVE_TYPE) {
        outputs = full;
      }  // else if (archive_type == UNKNOWN) see below

      for (const wchar_t* const* inputs : input_arrays) {
        if (archive_type == installer::UNKNOWN_ARCHIVE_TYPE) {
          // "-full" is untouched if the archive type is unknown.
          // "-multifail" is unconditionally removed.
          if (inputs == full || inputs == multifail_full)
            outputs = full;
          else
            outputs = plain;
        }
        for (size_t input_idx = 0; input_idx < arraysize(plain); ++input_idx) {
          const wchar_t* input = inputs[input_idx];
          const wchar_t* output = outputs[input_idx];

          v.set_value(input);
          if (output == v.value()) {
            EXPECT_FALSE(GoogleUpdateSettings::UpdateGoogleUpdateApKey(
                archive_type, result, &v))
                << "archive_type: " << archive_type
                << ", result: " << result
                << ", input ap value: " << input;
          } else {
            EXPECT_TRUE(GoogleUpdateSettings::UpdateGoogleUpdateApKey(
                archive_type, result, &v))
                << "archive_type: " << archive_type
                << ", result: " << result
                << ", input ap value: " << input;
          }
          EXPECT_EQ(output, v.value())
              << "archive_type: " << archive_type
              << ", result: " << result
              << ", input ap value: " << input;
        }
      }
    }
  }
}

TEST_F(GoogleUpdateSettingsTest, UpdateInstallStatusTest) {
  std::unique_ptr<WorkItemList> work_item_list(WorkItem::CreateWorkItemList());
  // Test incremental install failure
  ASSERT_TRUE(CreateApKey(work_item_list.get(), L""))
      << "Failed to create ap key.";
  GoogleUpdateSettings::UpdateInstallStatus(false,
                                            installer::INCREMENTAL_ARCHIVE_TYPE,
                                            installer::INSTALL_FAILED,
                                            kTestProductGuid);
  EXPECT_STREQ(ReadApKeyValue().c_str(), L"-full");
  work_item_list->Rollback();

  work_item_list.reset(WorkItem::CreateWorkItemList());
  // Test incremental install success
  ASSERT_TRUE(CreateApKey(work_item_list.get(), L""))
      << "Failed to create ap key.";
  GoogleUpdateSettings::UpdateInstallStatus(false,
                                            installer::INCREMENTAL_ARCHIVE_TYPE,
                                            installer::FIRST_INSTALL_SUCCESS,
                                            kTestProductGuid);
  EXPECT_STREQ(ReadApKeyValue().c_str(), L"");
  work_item_list->Rollback();

  work_item_list.reset(WorkItem::CreateWorkItemList());
  // Test full install failure
  ASSERT_TRUE(CreateApKey(work_item_list.get(), L"-full"))
      << "Failed to create ap key.";
  GoogleUpdateSettings::UpdateInstallStatus(false, installer::FULL_ARCHIVE_TYPE,
                                            installer::INSTALL_FAILED,
                                            kTestProductGuid);
  EXPECT_STREQ(ReadApKeyValue().c_str(), L"");
  work_item_list->Rollback();

  work_item_list.reset(WorkItem::CreateWorkItemList());
  // Test full install success
  ASSERT_TRUE(CreateApKey(work_item_list.get(), L"-full"))
      << "Failed to create ap key.";
  GoogleUpdateSettings::UpdateInstallStatus(false, installer::FULL_ARCHIVE_TYPE,
                                            installer::FIRST_INSTALL_SUCCESS,
                                            kTestProductGuid);
  EXPECT_STREQ(ReadApKeyValue().c_str(), L"");
  work_item_list->Rollback();

  work_item_list.reset(WorkItem::CreateWorkItemList());
  // Test the case of when "ap" key doesnt exist at all
  base::string16 ap_key_value = ReadApKeyValue();
  base::string16 reg_key = GetApKeyPath();
  HKEY reg_root = HKEY_CURRENT_USER;
  bool ap_key_deleted = false;
  RegKey key;
  if (key.Open(HKEY_CURRENT_USER, reg_key.c_str(), KEY_ALL_ACCESS) !=
      ERROR_SUCCESS) {
    work_item_list->AddCreateRegKeyWorkItem(
        reg_root, reg_key, WorkItem::kWow64Default);
    ASSERT_TRUE(work_item_list->Do()) << "Failed to create ClientState key.";
  } else if (key.DeleteValue(google_update::kRegApField) == ERROR_SUCCESS) {
    ap_key_deleted = true;
  }
  // try differential installer
  GoogleUpdateSettings::UpdateInstallStatus(false,
                                            installer::INCREMENTAL_ARCHIVE_TYPE,
                                            installer::INSTALL_FAILED,
                                            kTestProductGuid);
  EXPECT_STREQ(ReadApKeyValue().c_str(), L"-full");
  // try full installer now
  GoogleUpdateSettings::UpdateInstallStatus(false, installer::FULL_ARCHIVE_TYPE,
                                            installer::INSTALL_FAILED,
                                            kTestProductGuid);
  EXPECT_STREQ(ReadApKeyValue().c_str(), L"");
  // Now cleanup to leave the system in unchanged state.
  // - Diff installer creates an ap key if it didnt exist, so delete this ap key
  // - If we created any reg key path for ap, roll it back
  // - Finally restore the original value of ap key.
  key.Open(HKEY_CURRENT_USER, reg_key.c_str(), KEY_ALL_ACCESS);
  key.DeleteValue(google_update::kRegApField);
  work_item_list->Rollback();
  if (ap_key_deleted) {
    work_item_list.reset(WorkItem::CreateWorkItemList());
    ASSERT_TRUE(CreateApKey(work_item_list.get(), ap_key_value))
        << "Failed to restore ap key.";
  }
}

TEST_F(GoogleUpdateSettingsTest, SetEULAConsent) {
  using installer::FakeInstallationState;

  const bool system_level = true;
  FakeInstallationState machine_state;

  // Chrome is installed.
  machine_state.AddChrome(system_level,
                          new base::Version(chrome::kChromeVersion));

  RegKey key;
  DWORD value;
  BrowserDistribution* chrome = BrowserDistribution::GetDistribution();

  // eulaconsent is set on the product.
  EXPECT_TRUE(GoogleUpdateSettings::SetEULAConsent(machine_state, chrome,
                                                   true));
  EXPECT_EQ(ERROR_SUCCESS,
      key.Open(HKEY_LOCAL_MACHINE, chrome->GetStateMediumKey().c_str(),
               KEY_QUERY_VALUE));
  EXPECT_EQ(ERROR_SUCCESS,
      key.ReadValueDW(google_update::kRegEULAAceptedField, &value));
  EXPECT_EQ(1U, value);
}

// Test that the appropriate default is returned if no update override is
// present.
TEST_F(GoogleUpdateSettingsTest, GetAppUpdatePolicyNoOverride) {
  // There are no policies at all.
  EXPECT_EQ(ERROR_FILE_NOT_FOUND,
            RegKey().Open(HKEY_LOCAL_MACHINE,
                          GoogleUpdateSettings::kPoliciesKey,
                          KEY_QUERY_VALUE));
  bool is_overridden = true;
  EXPECT_EQ(GoogleUpdateSettings::kDefaultUpdatePolicy,
            GoogleUpdateSettings::GetAppUpdatePolicy(kTestProductGuid,
                                                     &is_overridden));
  EXPECT_FALSE(is_overridden);

  // The policy key exists, but there are no values of interest present.
  EXPECT_EQ(ERROR_SUCCESS,
            RegKey().Create(HKEY_LOCAL_MACHINE,
                            GoogleUpdateSettings::kPoliciesKey,
                            KEY_SET_VALUE));
  EXPECT_EQ(ERROR_SUCCESS,
            RegKey().Open(HKEY_LOCAL_MACHINE,
                          GoogleUpdateSettings::kPoliciesKey,
                          KEY_QUERY_VALUE));
  is_overridden = true;
  EXPECT_EQ(GoogleUpdateSettings::kDefaultUpdatePolicy,
            GoogleUpdateSettings::GetAppUpdatePolicy(kTestProductGuid,
                                                     &is_overridden));
  EXPECT_FALSE(is_overridden);
}

TEST_F(GoogleUpdateSettingsTest, UpdateProfileCountsSystemInstall) {
  // Set up a basic system-level InstallDetails.
  install_static::ScopedInstallDetails details(true /* system_level */);

  // No profile count keys present yet.
  const base::string16& state_key = BrowserDistribution::GetDistribution()->
      GetAppRegistrationData().GetStateMediumKey();
  base::string16 num_profiles_path(state_key);
  num_profiles_path.append(L"\\");
  num_profiles_path.append(google_update::kRegProfilesActive);
  base::string16 num_signed_in_path(state_key);
  num_signed_in_path.append(L"\\");
  num_signed_in_path.append(google_update::kRegProfilesSignedIn);

  EXPECT_EQ(ERROR_FILE_NOT_FOUND,
            RegKey().Open(HKEY_LOCAL_MACHINE,
                          num_profiles_path.c_str(),
                          KEY_QUERY_VALUE));
  EXPECT_EQ(ERROR_FILE_NOT_FOUND,
            RegKey().Open(HKEY_LOCAL_MACHINE,
                          num_signed_in_path.c_str(),
                          KEY_QUERY_VALUE));

  // Show time! Write the values.
  GoogleUpdateSettings::UpdateProfileCounts(3, 2);

  // Verify the keys were created.
  EXPECT_EQ(ERROR_SUCCESS,
            RegKey().Open(HKEY_LOCAL_MACHINE,
                          num_profiles_path.c_str(),
                          KEY_QUERY_VALUE));
  EXPECT_EQ(ERROR_SUCCESS,
            RegKey().Open(HKEY_LOCAL_MACHINE,
                          num_signed_in_path.c_str(),
                          KEY_QUERY_VALUE));

  base::string16 uniquename;
  EXPECT_TRUE(base::win::GetUserSidString(&uniquename));

  // Verify the values are accessible.
  DWORD num_profiles = 0;
  DWORD num_signed_in = 0;
  base::string16 aggregate;
  EXPECT_EQ(
      ERROR_SUCCESS,
      RegKey(HKEY_LOCAL_MACHINE, num_profiles_path.c_str(),
             KEY_QUERY_VALUE).ReadValueDW(uniquename.c_str(),
                                          &num_profiles));
  EXPECT_EQ(
      ERROR_SUCCESS,
      RegKey(HKEY_LOCAL_MACHINE, num_signed_in_path.c_str(),
             KEY_QUERY_VALUE).ReadValueDW(uniquename.c_str(),
                                          &num_signed_in));
  EXPECT_EQ(
      ERROR_SUCCESS,
      RegKey(HKEY_LOCAL_MACHINE, num_signed_in_path.c_str(),
             KEY_QUERY_VALUE).ReadValue(google_update::kRegAggregateMethod,
                                        &aggregate));

  // Verify the correct values were written.
  EXPECT_EQ(3u, num_profiles);
  EXPECT_EQ(2u, num_signed_in);
  EXPECT_EQ(L"sum()", aggregate);
}

TEST_F(GoogleUpdateSettingsTest, UpdateProfileCountsUserInstall) {
  // Unit tests never operate as an installed application, so will never
  // be a system install.

  // No profile count values present yet.
  const base::string16& state_key = BrowserDistribution::GetDistribution()->
      GetAppRegistrationData().GetStateKey();

  EXPECT_EQ(ERROR_FILE_NOT_FOUND,
            RegKey().Open(HKEY_CURRENT_USER,
                          state_key.c_str(),
                          KEY_QUERY_VALUE));

  // Show time! Write the values.
  GoogleUpdateSettings::UpdateProfileCounts(4, 1);

  // Verify the key was created.
  EXPECT_EQ(ERROR_SUCCESS,
            RegKey().Open(HKEY_CURRENT_USER,
                          state_key.c_str(),
                          KEY_QUERY_VALUE));

  // Verify the values are accessible.
  base::string16 num_profiles;
  base::string16 num_signed_in;
  EXPECT_EQ(
      ERROR_SUCCESS,
      RegKey(HKEY_CURRENT_USER, state_key.c_str(), KEY_QUERY_VALUE).
          ReadValue(google_update::kRegProfilesActive, &num_profiles));
  EXPECT_EQ(
      ERROR_SUCCESS,
      RegKey(HKEY_CURRENT_USER, state_key.c_str(), KEY_QUERY_VALUE).
          ReadValue(google_update::kRegProfilesSignedIn, &num_signed_in));

  // Verify the correct values were written.
  EXPECT_EQ(L"4", num_profiles);
  EXPECT_EQ(L"1", num_signed_in);
}

#if defined(GOOGLE_CHROME_BUILD)

// Test that the default override is returned if no app-specific override is
// present.
TEST_F(GoogleUpdateSettingsTest, GetAppUpdatePolicyDefaultOverride) {
  EXPECT_EQ(ERROR_SUCCESS,
            RegKey(HKEY_LOCAL_MACHINE, GoogleUpdateSettings::kPoliciesKey,
                   KEY_SET_VALUE).WriteValue(
                       GoogleUpdateSettings::kUpdatePolicyValue,
                       static_cast<DWORD>(0)));
  bool is_overridden = true;
  EXPECT_EQ(GoogleUpdateSettings::UPDATES_DISABLED,
            GoogleUpdateSettings::GetAppUpdatePolicy(kTestProductGuid,
                                                     &is_overridden));
  EXPECT_FALSE(is_overridden);

  EXPECT_EQ(ERROR_SUCCESS,
            RegKey(HKEY_LOCAL_MACHINE, GoogleUpdateSettings::kPoliciesKey,
                   KEY_SET_VALUE).WriteValue(
                       GoogleUpdateSettings::kUpdatePolicyValue,
                       static_cast<DWORD>(1)));
  is_overridden = true;
  EXPECT_EQ(GoogleUpdateSettings::AUTOMATIC_UPDATES,
            GoogleUpdateSettings::GetAppUpdatePolicy(kTestProductGuid,
                                                     &is_overridden));
  EXPECT_FALSE(is_overridden);

  EXPECT_EQ(ERROR_SUCCESS,
            RegKey(HKEY_LOCAL_MACHINE, GoogleUpdateSettings::kPoliciesKey,
                   KEY_SET_VALUE).WriteValue(
                       GoogleUpdateSettings::kUpdatePolicyValue,
                       static_cast<DWORD>(2)));
  is_overridden = true;
  EXPECT_EQ(GoogleUpdateSettings::MANUAL_UPDATES_ONLY,
            GoogleUpdateSettings::GetAppUpdatePolicy(kTestProductGuid,
                                                     &is_overridden));
  EXPECT_FALSE(is_overridden);

  EXPECT_EQ(ERROR_SUCCESS,
            RegKey(HKEY_LOCAL_MACHINE, GoogleUpdateSettings::kPoliciesKey,
                   KEY_SET_VALUE).WriteValue(
                       GoogleUpdateSettings::kUpdatePolicyValue,
                       static_cast<DWORD>(3)));
  is_overridden = true;
  EXPECT_EQ(GoogleUpdateSettings::AUTO_UPDATES_ONLY,
            GoogleUpdateSettings::GetAppUpdatePolicy(kTestProductGuid,
                                                     &is_overridden));
  EXPECT_FALSE(is_overridden);

  // The default policy should be in force for bogus values.
  EXPECT_EQ(ERROR_SUCCESS,
            RegKey(HKEY_LOCAL_MACHINE, GoogleUpdateSettings::kPoliciesKey,
                   KEY_SET_VALUE).WriteValue(
                       GoogleUpdateSettings::kUpdatePolicyValue,
                       static_cast<DWORD>(4)));
  is_overridden = true;
  EXPECT_EQ(GoogleUpdateSettings::kDefaultUpdatePolicy,
            GoogleUpdateSettings::GetAppUpdatePolicy(kTestProductGuid,
                                                     &is_overridden));
  EXPECT_FALSE(is_overridden);
}

// Test that an app-specific override is used if present.
TEST_F(GoogleUpdateSettingsTest, GetAppUpdatePolicyAppOverride) {
  base::string16 app_policy_value(
      GoogleUpdateSettings::kUpdateOverrideValuePrefix);
  app_policy_value.append(kTestProductGuid);

  EXPECT_EQ(ERROR_SUCCESS,
            RegKey(HKEY_LOCAL_MACHINE, GoogleUpdateSettings::kPoliciesKey,
                   KEY_SET_VALUE).WriteValue(
                       GoogleUpdateSettings::kUpdatePolicyValue,
                       static_cast<DWORD>(1)));
  EXPECT_EQ(ERROR_SUCCESS,
            RegKey(HKEY_LOCAL_MACHINE, GoogleUpdateSettings::kPoliciesKey,
                   KEY_SET_VALUE).WriteValue(app_policy_value.c_str(),
                                             static_cast<DWORD>(0)));
  bool is_overridden = false;
  EXPECT_EQ(GoogleUpdateSettings::UPDATES_DISABLED,
            GoogleUpdateSettings::GetAppUpdatePolicy(kTestProductGuid,
                                                     &is_overridden));
  EXPECT_TRUE(is_overridden);

  EXPECT_EQ(ERROR_SUCCESS,
            RegKey(HKEY_LOCAL_MACHINE, GoogleUpdateSettings::kPoliciesKey,
                   KEY_SET_VALUE).WriteValue(
                       GoogleUpdateSettings::kUpdatePolicyValue,
                       static_cast<DWORD>(0)));
  EXPECT_EQ(ERROR_SUCCESS,
            RegKey(HKEY_LOCAL_MACHINE, GoogleUpdateSettings::kPoliciesKey,
                   KEY_SET_VALUE).WriteValue(app_policy_value.c_str(),
                                             static_cast<DWORD>(1)));
  is_overridden = false;
  EXPECT_EQ(GoogleUpdateSettings::AUTOMATIC_UPDATES,
            GoogleUpdateSettings::GetAppUpdatePolicy(kTestProductGuid,
                                                     &is_overridden));
  EXPECT_TRUE(is_overridden);

  EXPECT_EQ(ERROR_SUCCESS,
            RegKey(HKEY_LOCAL_MACHINE, GoogleUpdateSettings::kPoliciesKey,
                   KEY_SET_VALUE).WriteValue(app_policy_value.c_str(),
                                             static_cast<DWORD>(2)));
  is_overridden = false;
  EXPECT_EQ(GoogleUpdateSettings::MANUAL_UPDATES_ONLY,
            GoogleUpdateSettings::GetAppUpdatePolicy(kTestProductGuid,
                                                     &is_overridden));
  EXPECT_TRUE(is_overridden);

  EXPECT_EQ(ERROR_SUCCESS,
            RegKey(HKEY_LOCAL_MACHINE, GoogleUpdateSettings::kPoliciesKey,
                   KEY_SET_VALUE).WriteValue(app_policy_value.c_str(),
                                             static_cast<DWORD>(3)));
  is_overridden = false;
  EXPECT_EQ(GoogleUpdateSettings::AUTO_UPDATES_ONLY,
            GoogleUpdateSettings::GetAppUpdatePolicy(kTestProductGuid,
                                                     &is_overridden));
  EXPECT_TRUE(is_overridden);

  // The default policy should be in force for bogus values.
  EXPECT_EQ(ERROR_SUCCESS,
            RegKey(HKEY_LOCAL_MACHINE, GoogleUpdateSettings::kPoliciesKey,
                   KEY_SET_VALUE).WriteValue(app_policy_value.c_str(),
                                             static_cast<DWORD>(4)));
  is_overridden = true;
  EXPECT_EQ(GoogleUpdateSettings::UPDATES_DISABLED,
            GoogleUpdateSettings::GetAppUpdatePolicy(kTestProductGuid,
                                                     &is_overridden));
  EXPECT_FALSE(is_overridden);
}

TEST_F(GoogleUpdateSettingsTest, PerAppUpdatesDisabledByPolicy) {
  const wchar_t* app_guid = install_static::GetAppGuid();
  EXPECT_TRUE(SetUpdatePolicyForAppGuid(
      app_guid, GoogleUpdateSettings::UPDATES_DISABLED));
  bool is_overridden = false;
  GoogleUpdateSettings::UpdatePolicy update_policy =
      GoogleUpdateSettings::GetAppUpdatePolicy(app_guid, &is_overridden);
  EXPECT_TRUE(is_overridden);
  EXPECT_EQ(GoogleUpdateSettings::UPDATES_DISABLED, update_policy);
  EXPECT_FALSE(GoogleUpdateSettings::AreAutoupdatesEnabled());

  EXPECT_TRUE(GoogleUpdateSettings::ReenableAutoupdates());
  update_policy =
      GoogleUpdateSettings::GetAppUpdatePolicy(app_guid, &is_overridden);
  // Should still have a policy but now that policy should explicitly enable
  // updates.
  EXPECT_TRUE(is_overridden);
  EXPECT_EQ(GoogleUpdateSettings::AUTOMATIC_UPDATES, update_policy);
  EXPECT_TRUE(GoogleUpdateSettings::AreAutoupdatesEnabled());
}

TEST_F(GoogleUpdateSettingsTest, PerAppUpdatesEnabledWithGlobalDisabled) {
  // Disable updates globally but enable them for Chrome (the app-specific
  // setting should take precedence).
  const wchar_t* app_guid = install_static::GetAppGuid();
  EXPECT_TRUE(SetUpdatePolicyForAppGuid(
      app_guid, GoogleUpdateSettings::AUTOMATIC_UPDATES));
  EXPECT_TRUE(SetGlobalUpdatePolicy(GoogleUpdateSettings::UPDATES_DISABLED));

  // Make sure we read this as still having updates enabled.
  EXPECT_TRUE(GoogleUpdateSettings::AreAutoupdatesEnabled());

  // Make sure that the reset action returns true and is a no-op.
  EXPECT_TRUE(GoogleUpdateSettings::ReenableAutoupdates());
  EXPECT_EQ(GoogleUpdateSettings::AUTOMATIC_UPDATES,
            GetUpdatePolicyForAppGuid(app_guid));
  EXPECT_EQ(GoogleUpdateSettings::UPDATES_DISABLED, GetGlobalUpdatePolicy());
}

TEST_F(GoogleUpdateSettingsTest, GlobalUpdatesDisabledByPolicy) {
  const wchar_t* app_guid = install_static::GetAppGuid();
  EXPECT_TRUE(SetGlobalUpdatePolicy(GoogleUpdateSettings::UPDATES_DISABLED));
  bool is_overridden = false;

  // The contract for GetAppUpdatePolicy states that |is_overridden| should be
  // set to false when updates are disabled on a non-app-specific basis.
  GoogleUpdateSettings::UpdatePolicy update_policy =
      GoogleUpdateSettings::GetAppUpdatePolicy(app_guid, &is_overridden);
  EXPECT_FALSE(is_overridden);
  EXPECT_EQ(GoogleUpdateSettings::UPDATES_DISABLED, update_policy);
  EXPECT_FALSE(GoogleUpdateSettings::AreAutoupdatesEnabled());

  EXPECT_TRUE(GoogleUpdateSettings::ReenableAutoupdates());
  update_policy =
      GoogleUpdateSettings::GetAppUpdatePolicy(app_guid, &is_overridden);
  // Policy should now be to enable updates, |is_overridden| should still be
  // false.
  EXPECT_FALSE(is_overridden);
  EXPECT_EQ(GoogleUpdateSettings::AUTOMATIC_UPDATES, update_policy);
  EXPECT_TRUE(GoogleUpdateSettings::AreAutoupdatesEnabled());
}

TEST_F(GoogleUpdateSettingsTest, UpdatesDisabledByTimeout) {
  // Disable updates altogether.
  EXPECT_TRUE(SetUpdateTimeoutOverride(0));
  EXPECT_FALSE(GoogleUpdateSettings::AreAutoupdatesEnabled());
  EXPECT_TRUE(GoogleUpdateSettings::ReenableAutoupdates());
  EXPECT_TRUE(GoogleUpdateSettings::AreAutoupdatesEnabled());

  // Set the update period to something unreasonable.
  EXPECT_TRUE(SetUpdateTimeoutOverride(
      GoogleUpdateSettings::kCheckPeriodOverrideMinutesMax + 1));
  EXPECT_FALSE(GoogleUpdateSettings::AreAutoupdatesEnabled());
  EXPECT_TRUE(GoogleUpdateSettings::ReenableAutoupdates());
  EXPECT_TRUE(GoogleUpdateSettings::AreAutoupdatesEnabled());
}

#endif  // defined(GOOGLE_CHROME_BUILD)

TEST_F(GoogleUpdateSettingsTest, ExperimentsLabelHelperSystem) {
  TestExperimentsLabelHelper(SYSTEM_INSTALL);
}

TEST_F(GoogleUpdateSettingsTest, ExperimentsLabelHelperUser) {
  TestExperimentsLabelHelper(USER_INSTALL);
}

TEST_F(GoogleUpdateSettingsTest, GetDownloadPreference) {
  RegKey policy_key;

  if (policy_key.Open(HKEY_LOCAL_MACHINE, GoogleUpdateSettings::kPoliciesKey,
                      KEY_SET_VALUE) == ERROR_SUCCESS) {
    policy_key.DeleteValue(
        GoogleUpdateSettings::kDownloadPreferencePolicyValue);
  }
  policy_key.Close();

  // When no policy is present expect to return an empty string.
  EXPECT_TRUE(GoogleUpdateSettings::GetDownloadPreference().empty());

  // Expect "cacheable" when the correct policy is present.
  EXPECT_EQ(ERROR_SUCCESS, policy_key.Create(HKEY_LOCAL_MACHINE,
                                             GoogleUpdateSettings::kPoliciesKey,
                                             KEY_SET_VALUE));
  EXPECT_EQ(
      ERROR_SUCCESS,
      policy_key.WriteValue(
          GoogleUpdateSettings::kDownloadPreferencePolicyValue, L"cacheable"));
  EXPECT_STREQ(L"cacheable",
               GoogleUpdateSettings::GetDownloadPreference().c_str());

  EXPECT_EQ(ERROR_SUCCESS,
            policy_key.WriteValue(
                GoogleUpdateSettings::kDownloadPreferencePolicyValue,
                base::string16(32, L'a').c_str()));
  EXPECT_STREQ(base::string16(32, L'a').c_str(),
               GoogleUpdateSettings::GetDownloadPreference().c_str());

  // Expect an empty string when an unsupported policy is set.
  // It contains spaces.
  EXPECT_EQ(ERROR_SUCCESS,
            policy_key.WriteValue(
                GoogleUpdateSettings::kDownloadPreferencePolicyValue, L"a b"));
  EXPECT_TRUE(GoogleUpdateSettings::GetDownloadPreference().empty());

  // It contains non alpha-numeric characters.
  EXPECT_EQ(ERROR_SUCCESS,
            policy_key.WriteValue(
                GoogleUpdateSettings::kDownloadPreferencePolicyValue, L"<a>"));
  EXPECT_TRUE(GoogleUpdateSettings::GetDownloadPreference().empty());

  // It is too long.
  EXPECT_EQ(ERROR_SUCCESS,
            policy_key.WriteValue(
                GoogleUpdateSettings::kDownloadPreferencePolicyValue,
                base::string16(33, L'a').c_str()));
  EXPECT_TRUE(GoogleUpdateSettings::GetDownloadPreference().empty());
}

class SetProgressTest : public GoogleUpdateSettingsTest,
                        public testing::WithParamInterface<bool> {
 protected:
  SetProgressTest()
      : system_install_(GetParam()),
        root_key_(system_install_ ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER) {}

  const bool system_install_;
  const HKEY root_key_;
};

TEST_P(SetProgressTest, SetProgress) {
  base::string16 path(google_update::kRegPathClientState);
  path += L"\\";
  path += kTestProductGuid;

  constexpr int kValues[] = {0, 25, 50, 99, 100};
  for (int value : kValues) {
    GoogleUpdateSettings::SetProgress(system_install_, path, value);
    DWORD progress = 0;
    base::win::RegKey key(root_key_, path.c_str(),
                          KEY_QUERY_VALUE | KEY_WOW64_32KEY);
    ASSERT_TRUE(key.Valid());
    ASSERT_EQ(ERROR_SUCCESS,
              key.ReadValueDW(google_update::kRegInstallerProgress, &progress));
    EXPECT_EQ(static_cast<DWORD>(value), progress);
  }
}

INSTANTIATE_TEST_CASE_P(SetProgressUserLevel,
                        SetProgressTest,
                        testing::Values(false));
INSTANTIATE_TEST_CASE_P(SetProgressSystemLevel,
                        SetProgressTest,
                        testing::Values(true));

// Test GoogleUpdateSettings::GetUninstallCommandLine at system- or user-level,
// according to the param.
class GetUninstallCommandLine : public GoogleUpdateSettingsTest,
                                public testing::WithParamInterface<bool> {
 protected:
  static const wchar_t kDummyCommand[];

  void SetUp() override {
    GoogleUpdateSettingsTest::SetUp();
    system_install_ = GetParam();
    root_key_ = system_install_ ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
  }

  HKEY root_key_;
  bool system_install_;
};

const wchar_t GetUninstallCommandLine::kDummyCommand[] =
    L"\"goopdate.exe\" /spam";

// Tests that GetUninstallCommandLine returns an empty string if there's no
// Software\Google\Update key.
TEST_P(GetUninstallCommandLine, TestNoKey) {
  EXPECT_EQ(base::string16(),
            GoogleUpdateSettings::GetUninstallCommandLine(system_install_));
}

// Tests that GetUninstallCommandLine returns an empty string if there's no
// UninstallCmdLine value in the Software\Google\Update key.
TEST_P(GetUninstallCommandLine, TestNoValue) {
  RegKey(root_key_, google_update::kRegPathGoogleUpdate, KEY_SET_VALUE);
  EXPECT_EQ(base::string16(),
            GoogleUpdateSettings::GetUninstallCommandLine(system_install_));
}

// Tests that GetUninstallCommandLine returns an empty string if there's an
// empty UninstallCmdLine value in the Software\Google\Update key.
TEST_P(GetUninstallCommandLine, TestEmptyValue) {
  RegKey(root_key_, google_update::kRegPathGoogleUpdate, KEY_SET_VALUE)
    .WriteValue(google_update::kRegUninstallCmdLine, L"");
  EXPECT_EQ(base::string16(),
            GoogleUpdateSettings::GetUninstallCommandLine(system_install_));
}

// Tests that GetUninstallCommandLine returns the correct string if there's an
// UninstallCmdLine value in the Software\Google\Update key.
TEST_P(GetUninstallCommandLine, TestRealValue) {
  RegKey(root_key_, google_update::kRegPathGoogleUpdate, KEY_SET_VALUE)
      .WriteValue(google_update::kRegUninstallCmdLine, kDummyCommand);
  EXPECT_EQ(base::string16(kDummyCommand),
            GoogleUpdateSettings::GetUninstallCommandLine(system_install_));
  // Make sure that there's no value in the other level (user or system).
  EXPECT_EQ(base::string16(),
            GoogleUpdateSettings::GetUninstallCommandLine(!system_install_));
}

INSTANTIATE_TEST_CASE_P(GetUninstallCommandLineAtLevel, GetUninstallCommandLine,
                        testing::Bool());

// Test GoogleUpdateSettings::GetGoogleUpdateVersion at system- or user-level,
// according to the param.
class GetGoogleUpdateVersion : public GoogleUpdateSettingsTest,
                               public testing::WithParamInterface<bool> {
 protected:
  static const wchar_t kDummyVersion[];

  void SetUp() override {
    GoogleUpdateSettingsTest::SetUp();
    system_install_ = GetParam();
    root_key_ = system_install_ ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
  }

  HKEY root_key_;
  bool system_install_;
};

const wchar_t GetGoogleUpdateVersion::kDummyVersion[] = L"1.2.3.4";

// Tests that GetGoogleUpdateVersion returns an empty string if there's no
// Software\Google\Update key.
TEST_P(GetGoogleUpdateVersion, TestNoKey) {
  EXPECT_FALSE(
      GoogleUpdateSettings::GetGoogleUpdateVersion(system_install_).IsValid());
}

// Tests that GetGoogleUpdateVersion returns an empty string if there's no
// version value in the Software\Google\Update key.
TEST_P(GetGoogleUpdateVersion, TestNoValue) {
  RegKey(root_key_, google_update::kRegPathGoogleUpdate, KEY_SET_VALUE);
  EXPECT_FALSE(
      GoogleUpdateSettings::GetGoogleUpdateVersion(system_install_).IsValid());
}

// Tests that GetGoogleUpdateVersion returns an empty string if there's an
// empty version value in the Software\Google\Update key.
TEST_P(GetGoogleUpdateVersion, TestEmptyValue) {
  RegKey(root_key_, google_update::kRegPathGoogleUpdate, KEY_SET_VALUE)
      .WriteValue(google_update::kRegGoogleUpdateVersion, L"");
  EXPECT_FALSE(
      GoogleUpdateSettings::GetGoogleUpdateVersion(system_install_).IsValid());
}

// Tests that GetGoogleUpdateVersion returns the correct string if there's a
// version value in the Software\Google\Update key.
TEST_P(GetGoogleUpdateVersion, TestRealValue) {
  RegKey(root_key_, google_update::kRegPathGoogleUpdate, KEY_SET_VALUE)
      .WriteValue(google_update::kRegGoogleUpdateVersion, kDummyVersion);
  base::Version expected(base::UTF16ToUTF8(kDummyVersion));
  EXPECT_EQ(expected,
      GoogleUpdateSettings::GetGoogleUpdateVersion(system_install_));
  // Make sure that there's no value in the other level (user or system).
  EXPECT_FALSE(
      GoogleUpdateSettings::GetGoogleUpdateVersion(!system_install_)
          .IsValid());
}

INSTANTIATE_TEST_CASE_P(GetGoogleUpdateVersionAtLevel, GetGoogleUpdateVersion,
                        testing::Bool());

// Test values for use by the CollectStatsConsent test fixture.
class StatsState {
 public:
  enum StateSetting {
    NO_SETTING,
    FALSE_SETTING,
    TRUE_SETTING,
  };
  struct UserLevelState {};
  struct SystemLevelState {};
  static const UserLevelState kUserLevel;
  static const SystemLevelState kSystemLevel;

  StatsState(const UserLevelState&,
             StateSetting state_value)
      : system_level_(false),
        state_value_(state_value),
        state_medium_value_(NO_SETTING) {
  }
  StatsState(const SystemLevelState&,
             StateSetting state_value,
             StateSetting state_medium_value)
      : system_level_(true),
        state_value_(state_value),
        state_medium_value_(state_medium_value) {
  }
  bool system_level() const { return system_level_; }
  HKEY root_key() const {
    return system_level_ ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
  }
  StateSetting state_value() const { return state_value_; }
  StateSetting state_medium_value() const {
    return state_medium_value_;
  }
  bool is_consent_granted() const {
    return (system_level_ && state_medium_value_ != NO_SETTING) ?
        (state_medium_value_ == TRUE_SETTING) :
        (state_value_ == TRUE_SETTING);
  }

 private:
  bool system_level_;
  StateSetting state_value_;
  StateSetting state_medium_value_;
};

const StatsState::UserLevelState StatsState::kUserLevel = {};
const StatsState::SystemLevelState StatsState::kSystemLevel = {};

// A value parameterized test for testing the stats collection consent setting.
class CollectStatsConsent : public ::testing::TestWithParam<StatsState> {
 protected:
  CollectStatsConsent();
  void SetUp() override;
  void ApplySetting(StatsState::StateSetting setting,
                    HKEY root_key,
                    const base::string16& reg_key);

  BrowserDistribution* const dist_;
  registry_util::RegistryOverrideManager override_manager_;
  std::unique_ptr<install_static::ScopedInstallDetails> scoped_install_details_;
};

CollectStatsConsent::CollectStatsConsent()
    : dist_(BrowserDistribution::GetDistribution()) {}

// Install the registry override and apply the settings to the registry.
void CollectStatsConsent::SetUp() {
  // Override both HKLM and HKCU as tests may touch either/both.
  ASSERT_NO_FATAL_FAILURE(
      override_manager_.OverrideRegistry(HKEY_LOCAL_MACHINE));
  ASSERT_NO_FATAL_FAILURE(
      override_manager_.OverrideRegistry(HKEY_CURRENT_USER));

  const StatsState& stats_state = GetParam();
  scoped_install_details_ =
      base::MakeUnique<install_static::ScopedInstallDetails>(
          stats_state.system_level(), 0 /* install_mode_index */);
  const HKEY root_key = stats_state.root_key();
  ASSERT_NO_FATAL_FAILURE(
      ApplySetting(stats_state.state_value(), root_key, dist_->GetStateKey()));
  ASSERT_NO_FATAL_FAILURE(ApplySetting(stats_state.state_medium_value(),
                                       root_key, dist_->GetStateMediumKey()));
}

// Write the correct value to represent |setting| in the registry.
void CollectStatsConsent::ApplySetting(StatsState::StateSetting setting,
                                       HKEY root_key,
                                       const base::string16& reg_key) {
  if (setting != StatsState::NO_SETTING) {
    DWORD value = setting != StatsState::FALSE_SETTING ? 1 : 0;
    ASSERT_EQ(
        ERROR_SUCCESS,
        RegKey(root_key, reg_key.c_str(),
               KEY_SET_VALUE).WriteValue(google_update::kRegUsageStatsField,
                                         value));
  }
}

// Test that stats consent can be read.
TEST_P(CollectStatsConsent, GetCollectStatsConsentAtLevel) {
  if (GetParam().is_consent_granted()) {
    EXPECT_TRUE(GoogleUpdateSettings::GetCollectStatsConsentAtLevel(
                    GetParam().system_level()));
  } else {
    EXPECT_FALSE(GoogleUpdateSettings::GetCollectStatsConsentAtLevel(
                     GetParam().system_level()));
  }
}

// Test that stats consent can be flipped to the opposite setting, that the new
// setting takes affect, and that the correct registry location is modified.
TEST_P(CollectStatsConsent, SetCollectStatsConsentAtLevel) {
  // When testing revoking consent, verify that backup client info is cleared.
  // To do so, first add some backup client info.
  if (GetParam().is_consent_granted()) {
    metrics::ClientInfo client_info;
    client_info.client_id = "01234567-89ab-cdef-fedc-ba9876543210";
    client_info.installation_date = 123;
    client_info.reporting_enabled_date = 345;
    GoogleUpdateSettings::StoreMetricsClientInfo(client_info);
  }

  EXPECT_TRUE(GoogleUpdateSettings::SetCollectStatsConsentAtLevel(
                  GetParam().system_level(),
                  !GetParam().is_consent_granted()));

  const base::string16 reg_key = GetParam().system_level()
                                     ? dist_->GetStateMediumKey()
                                     : dist_->GetStateKey();
  DWORD value = 0;
  EXPECT_EQ(
      ERROR_SUCCESS,
      RegKey(GetParam().root_key(), reg_key.c_str(),
             KEY_QUERY_VALUE).ReadValueDW(google_update::kRegUsageStatsField,
                                          &value));
  if (GetParam().is_consent_granted()) {
    EXPECT_FALSE(GoogleUpdateSettings::GetCollectStatsConsentAtLevel(
                     GetParam().system_level()));
    EXPECT_EQ(0UL, value);
  } else {
    EXPECT_TRUE(GoogleUpdateSettings::GetCollectStatsConsentAtLevel(
                    GetParam().system_level()));
    EXPECT_EQ(1UL, value);
    // Verify that backup client info has been cleared.
    EXPECT_FALSE(GoogleUpdateSettings::LoadMetricsClientInfo());
  }
}

INSTANTIATE_TEST_CASE_P(
    UserLevel,
    CollectStatsConsent,
    ::testing::Values(
        StatsState(StatsState::kUserLevel, StatsState::NO_SETTING),
        StatsState(StatsState::kUserLevel, StatsState::FALSE_SETTING),
        StatsState(StatsState::kUserLevel, StatsState::TRUE_SETTING)));
INSTANTIATE_TEST_CASE_P(
    SystemLevel,
    CollectStatsConsent,
    ::testing::Values(StatsState(StatsState::kSystemLevel,
                                 StatsState::NO_SETTING,
                                 StatsState::NO_SETTING),
                      StatsState(StatsState::kSystemLevel,
                                 StatsState::NO_SETTING,
                                 StatsState::FALSE_SETTING),
                      StatsState(StatsState::kSystemLevel,
                                 StatsState::NO_SETTING,
                                 StatsState::TRUE_SETTING),
                      StatsState(StatsState::kSystemLevel,
                                 StatsState::FALSE_SETTING,
                                 StatsState::NO_SETTING),
                      StatsState(StatsState::kSystemLevel,
                                 StatsState::FALSE_SETTING,
                                 StatsState::FALSE_SETTING),
                      StatsState(StatsState::kSystemLevel,
                                 StatsState::FALSE_SETTING,
                                 StatsState::TRUE_SETTING),
                      StatsState(StatsState::kSystemLevel,
                                 StatsState::TRUE_SETTING,
                                 StatsState::NO_SETTING),
                      StatsState(StatsState::kSystemLevel,
                                 StatsState::TRUE_SETTING,
                                 StatsState::FALSE_SETTING),
                      StatsState(StatsState::kSystemLevel,
                                 StatsState::TRUE_SETTING,
                                 StatsState::TRUE_SETTING)));

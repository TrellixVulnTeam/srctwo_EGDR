// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/installer/util/install_util.h"

#include <memory>
#include <string>
#include <utility>

#include "base/base_paths.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/path_service.h"
#include "base/strings/string_util.h"
#include "base/test/scoped_path_override.h"
#include "base/test/test_reg_util_win.h"
#include "base/version.h"
#include "base/win/registry.h"
#include "chrome/installer/util/google_update_constants.h"
#include "chrome/installer/util/test_app_registration_data.h"
#include "chrome/installer/util/work_item.h"
#include "chrome/installer/util/work_item_list.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using base::win::RegKey;
using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

class MockRegistryValuePredicate : public InstallUtil::RegistryValuePredicate {
 public:
  MOCK_CONST_METHOD1(Evaluate, bool(const std::wstring&));
};

class TestBrowserDistribution : public BrowserDistribution {
 public:
  TestBrowserDistribution()
      : BrowserDistribution(base::MakeUnique<TestAppRegistrationData>()) {}
};

class InstallUtilTest : public testing::Test {
 protected:
  InstallUtilTest() {}

  void SetUp() override { ASSERT_NO_FATAL_FAILURE(ResetRegistryOverrides()); }

  void ResetRegistryOverrides() {
    registry_override_manager_.reset(
        new registry_util::RegistryOverrideManager);
    ASSERT_NO_FATAL_FAILURE(
        registry_override_manager_->OverrideRegistry(HKEY_CURRENT_USER));
    ASSERT_NO_FATAL_FAILURE(
        registry_override_manager_->OverrideRegistry(HKEY_LOCAL_MACHINE));
  }

 private:
  std::unique_ptr<registry_util::RegistryOverrideManager>
      registry_override_manager_;

  DISALLOW_COPY_AND_ASSIGN(InstallUtilTest);
};

TEST_F(InstallUtilTest, ComposeCommandLine) {
  base::CommandLine command_line(base::CommandLine::NO_PROGRAM);

  std::pair<std::wstring, std::wstring> params[] = {
    std::make_pair(std::wstring(L""), std::wstring(L"")),
    std::make_pair(std::wstring(L""), std::wstring(L"--do-something --silly")),
    std::make_pair(std::wstring(L"spam.exe"), std::wstring(L"")),
    std::make_pair(std::wstring(L"spam.exe"),
                   std::wstring(L"--do-something --silly")),
  };
  for (std::pair<std::wstring, std::wstring>& param : params) {
    InstallUtil::ComposeCommandLine(param.first, param.second, &command_line);
    EXPECT_EQ(param.first, command_line.GetProgram().value());
    if (param.second.empty()) {
      EXPECT_TRUE(command_line.GetSwitches().empty());
    } else {
      EXPECT_EQ(2U, command_line.GetSwitches().size());
      EXPECT_TRUE(command_line.HasSwitch("do-something"));
      EXPECT_TRUE(command_line.HasSwitch("silly"));
    }
  }
}

TEST_F(InstallUtilTest, GetCurrentDate) {
  std::wstring date(InstallUtil::GetCurrentDate());
  EXPECT_EQ(8u, date.length());
  if (date.length() == 8) {
    // For an invalid date value, SystemTimeToFileTime will fail.
    // We use this to validate that we have a correct date string.
    SYSTEMTIME systime = {0};
    FILETIME ft = {0};
    // Just to make sure our assumption holds.
    EXPECT_FALSE(SystemTimeToFileTime(&systime, &ft));
    // Now fill in the values from our string.
    systime.wYear = _wtoi(date.substr(0, 4).c_str());
    systime.wMonth = _wtoi(date.substr(4, 2).c_str());
    systime.wDay = _wtoi(date.substr(6, 2).c_str());
    // Check if they make sense.
    EXPECT_TRUE(SystemTimeToFileTime(&systime, &ft));
  }
}

TEST_F(InstallUtilTest, DeleteRegistryKeyIf) {
  const HKEY root = HKEY_CURRENT_USER;
  std::wstring parent_key_path(L"SomeKey\\ToDelete");
  std::wstring child_key_path(parent_key_path);
  child_key_path.append(L"\\ChildKey\\WithAValue");
  const wchar_t value_name[] = L"some_value_name";
  const wchar_t value[] = L"hi mom";

  // Nothing to delete if the keys aren't even there.
  {
    MockRegistryValuePredicate pred;

    EXPECT_CALL(pred, Evaluate(_)).Times(0);
    ASSERT_FALSE(
        RegKey(root, parent_key_path.c_str(), KEY_QUERY_VALUE).Valid());
    EXPECT_EQ(InstallUtil::NOT_FOUND,
              InstallUtil::DeleteRegistryKeyIf(
                  root, parent_key_path, child_key_path,
                  WorkItem::kWow64Default, value_name, pred));
    EXPECT_FALSE(
        RegKey(root, parent_key_path.c_str(), KEY_QUERY_VALUE).Valid());
  }

  // Parent exists, but not child: no delete.
  {
    MockRegistryValuePredicate pred;

    EXPECT_CALL(pred, Evaluate(_)).Times(0);
    ASSERT_TRUE(RegKey(root, parent_key_path.c_str(), KEY_SET_VALUE).Valid());
    EXPECT_EQ(InstallUtil::NOT_FOUND,
              InstallUtil::DeleteRegistryKeyIf(
                  root, parent_key_path, child_key_path,
                  WorkItem::kWow64Default, value_name, pred));
    EXPECT_TRUE(RegKey(root, parent_key_path.c_str(), KEY_QUERY_VALUE).Valid());
  }

  // Child exists, but no value: no delete.
  {
    MockRegistryValuePredicate pred;

    EXPECT_CALL(pred, Evaluate(_)).Times(0);
    ASSERT_TRUE(RegKey(root, child_key_path.c_str(), KEY_SET_VALUE).Valid());
    EXPECT_EQ(InstallUtil::NOT_FOUND,
              InstallUtil::DeleteRegistryKeyIf(
                  root, parent_key_path, child_key_path,
                  WorkItem::kWow64Default, value_name, pred));
    EXPECT_TRUE(RegKey(root, parent_key_path.c_str(), KEY_QUERY_VALUE).Valid());
  }

  // Value exists, but doesn't match: no delete.
  {
    MockRegistryValuePredicate pred;

    EXPECT_CALL(pred, Evaluate(StrEq(L"foosball!"))).WillOnce(Return(false));
    ASSERT_EQ(ERROR_SUCCESS, RegKey(root, child_key_path.c_str(), KEY_SET_VALUE)
                                 .WriteValue(value_name, L"foosball!"));
    EXPECT_EQ(InstallUtil::NOT_FOUND,
              InstallUtil::DeleteRegistryKeyIf(
                  root, parent_key_path, child_key_path,
                  WorkItem::kWow64Default, value_name, pred));
    EXPECT_TRUE(RegKey(root, parent_key_path.c_str(), KEY_QUERY_VALUE).Valid());
  }

  // Value exists, and matches: delete.
  {
    MockRegistryValuePredicate pred;

    EXPECT_CALL(pred, Evaluate(StrEq(value))).WillOnce(Return(true));
    ASSERT_EQ(ERROR_SUCCESS, RegKey(root, child_key_path.c_str(), KEY_SET_VALUE)
                                 .WriteValue(value_name, value));
    EXPECT_EQ(InstallUtil::DELETED,
              InstallUtil::DeleteRegistryKeyIf(
                  root, parent_key_path, child_key_path,
                  WorkItem::kWow64Default, value_name, pred));
    EXPECT_FALSE(
        RegKey(root, parent_key_path.c_str(), KEY_QUERY_VALUE).Valid());
  }

  // Default value exists and matches: delete.
  {
    MockRegistryValuePredicate pred;

    EXPECT_CALL(pred, Evaluate(StrEq(value))).WillOnce(Return(true));
    ASSERT_EQ(ERROR_SUCCESS, RegKey(root, child_key_path.c_str(), KEY_SET_VALUE)
                                 .WriteValue(NULL, value));
    EXPECT_EQ(InstallUtil::DELETED, InstallUtil::DeleteRegistryKeyIf(
                                        root, parent_key_path, child_key_path,
                                        WorkItem::kWow64Default, NULL, pred));
    EXPECT_FALSE(
        RegKey(root, parent_key_path.c_str(), KEY_QUERY_VALUE).Valid());
  }
}

TEST_F(InstallUtilTest, DeleteRegistryValueIf) {
  const HKEY root = HKEY_CURRENT_USER;
  std::wstring key_path(L"SomeKey\\ToDelete");
  const wchar_t value_name[] = L"some_value_name";
  const wchar_t value[] = L"hi mom";

  {
    ASSERT_NO_FATAL_FAILURE(ResetRegistryOverrides());
    // Nothing to delete if the key isn't even there.
    {
      MockRegistryValuePredicate pred;

      EXPECT_CALL(pred, Evaluate(_)).Times(0);
      ASSERT_FALSE(RegKey(root, key_path.c_str(), KEY_QUERY_VALUE).Valid());
      EXPECT_EQ(InstallUtil::NOT_FOUND,
                InstallUtil::DeleteRegistryValueIf(root, key_path.c_str(),
                                                   WorkItem::kWow64Default,
                                                   value_name, pred));
      EXPECT_FALSE(RegKey(root, key_path.c_str(), KEY_QUERY_VALUE).Valid());
    }

    // Key exists, but no value: no delete.
    {
      MockRegistryValuePredicate pred;

      EXPECT_CALL(pred, Evaluate(_)).Times(0);
      ASSERT_TRUE(RegKey(root, key_path.c_str(), KEY_SET_VALUE).Valid());
      EXPECT_EQ(InstallUtil::NOT_FOUND,
                InstallUtil::DeleteRegistryValueIf(root, key_path.c_str(),
                                                   WorkItem::kWow64Default,
                                                   value_name, pred));
      EXPECT_TRUE(RegKey(root, key_path.c_str(), KEY_QUERY_VALUE).Valid());
    }

    // Value exists, but doesn't match: no delete.
    {
      MockRegistryValuePredicate pred;

      EXPECT_CALL(pred, Evaluate(StrEq(L"foosball!"))).WillOnce(Return(false));
      ASSERT_EQ(ERROR_SUCCESS,
                RegKey(root, key_path.c_str(),
                       KEY_SET_VALUE).WriteValue(value_name, L"foosball!"));
      EXPECT_EQ(InstallUtil::NOT_FOUND,
                InstallUtil::DeleteRegistryValueIf(root, key_path.c_str(),
                                                   WorkItem::kWow64Default,
                                                   value_name, pred));
      EXPECT_TRUE(RegKey(root, key_path.c_str(), KEY_QUERY_VALUE).Valid());
      EXPECT_TRUE(RegKey(root, key_path.c_str(),
                         KEY_QUERY_VALUE).HasValue(value_name));
    }

    // Value exists, and matches: delete.
    {
      MockRegistryValuePredicate pred;

      EXPECT_CALL(pred, Evaluate(StrEq(value))).WillOnce(Return(true));
      ASSERT_EQ(ERROR_SUCCESS,
                RegKey(root, key_path.c_str(),
                       KEY_SET_VALUE).WriteValue(value_name, value));
      EXPECT_EQ(InstallUtil::DELETED,
                InstallUtil::DeleteRegistryValueIf(root, key_path.c_str(),
                                                   WorkItem::kWow64Default,
                                                   value_name, pred));
      EXPECT_TRUE(RegKey(root, key_path.c_str(), KEY_QUERY_VALUE).Valid());
      EXPECT_FALSE(RegKey(root, key_path.c_str(),
                          KEY_QUERY_VALUE).HasValue(value_name));
    }
  }

  {
    ASSERT_NO_FATAL_FAILURE(ResetRegistryOverrides());
    // Default value matches: delete using empty string.
    {
      MockRegistryValuePredicate pred;

      EXPECT_CALL(pred, Evaluate(StrEq(value))).WillOnce(Return(true));
      ASSERT_EQ(ERROR_SUCCESS,
                RegKey(root, key_path.c_str(),
                       KEY_SET_VALUE).WriteValue(L"", value));
      EXPECT_EQ(InstallUtil::DELETED,
                InstallUtil::DeleteRegistryValueIf(root, key_path.c_str(),
                                                   WorkItem::kWow64Default, L"",
                                                   pred));
      EXPECT_TRUE(RegKey(root, key_path.c_str(), KEY_QUERY_VALUE).Valid());
      EXPECT_FALSE(RegKey(root, key_path.c_str(),
                          KEY_QUERY_VALUE).HasValue(L""));
    }
  }

  {
    ASSERT_NO_FATAL_FAILURE(ResetRegistryOverrides());
    // Default value matches: delete using NULL.
    {
      MockRegistryValuePredicate pred;

      EXPECT_CALL(pred, Evaluate(StrEq(value))).WillOnce(Return(true));
      ASSERT_EQ(ERROR_SUCCESS,
                RegKey(root, key_path.c_str(),
                       KEY_SET_VALUE).WriteValue(L"", value));
      EXPECT_EQ(InstallUtil::DELETED,
                InstallUtil::DeleteRegistryValueIf(root, key_path.c_str(),
                                                   WorkItem::kWow64Default,
                                                   NULL, pred));
      EXPECT_TRUE(RegKey(root, key_path.c_str(), KEY_QUERY_VALUE).Valid());
      EXPECT_FALSE(RegKey(root, key_path.c_str(),
                          KEY_QUERY_VALUE).HasValue(L""));
    }
  }
}

TEST_F(InstallUtilTest, ValueEquals) {
  InstallUtil::ValueEquals pred(L"howdy");

  EXPECT_FALSE(pred.Evaluate(L""));
  EXPECT_FALSE(pred.Evaluate(L"Howdy"));
  EXPECT_FALSE(pred.Evaluate(L"howdy!"));
  EXPECT_FALSE(pred.Evaluate(L"!howdy"));
  EXPECT_TRUE(pred.Evaluate(L"howdy"));
}

TEST_F(InstallUtilTest, ProgramCompare) {
  base::ScopedTempDir test_dir;
  ASSERT_TRUE(test_dir.CreateUniqueTempDir());
  const base::FilePath some_long_dir(
      test_dir.GetPath().Append(L"Some Long Directory Name"));
  const base::FilePath expect(some_long_dir.Append(L"file.txt"));
  const base::FilePath expect_upcase(some_long_dir.Append(L"FILE.txt"));
  const base::FilePath other(some_long_dir.Append(L"otherfile.txt"));

  // Tests where the expected file doesn't exist.

  // Paths don't match.
  EXPECT_FALSE(InstallUtil::ProgramCompare(expect).Evaluate(
      L"\"" + other.value() + L"\""));
  // Paths match exactly.
  EXPECT_TRUE(InstallUtil::ProgramCompare(expect).Evaluate(
      L"\"" + expect.value() + L"\""));
  // Paths differ by case.
  EXPECT_TRUE(InstallUtil::ProgramCompare(expect).Evaluate(
      L"\"" + expect_upcase.value() + L"\""));

  // Tests where the expected file exists.
  static const char data[] = "data";
  ASSERT_TRUE(base::CreateDirectory(some_long_dir));
  ASSERT_NE(-1, base::WriteFile(expect, data, arraysize(data) - 1));
  // Paths don't match.
  EXPECT_FALSE(InstallUtil::ProgramCompare(expect).Evaluate(
      L"\"" + other.value() + L"\""));
  // Paths match exactly.
  EXPECT_TRUE(InstallUtil::ProgramCompare(expect).Evaluate(
      L"\"" + expect.value() + L"\""));
  // Paths differ by case.
  EXPECT_TRUE(InstallUtil::ProgramCompare(expect).Evaluate(
      L"\"" + expect_upcase.value() + L"\""));

  // Test where strings don't match, but the same file is indicated.
  std::wstring short_expect;
  DWORD short_len = GetShortPathName(expect.value().c_str(),
                                     base::WriteInto(&short_expect, MAX_PATH),
                                     MAX_PATH);
  ASSERT_NE(static_cast<DWORD>(0), short_len);
  ASSERT_GT(static_cast<DWORD>(MAX_PATH), short_len);
  short_expect.resize(short_len);
  ASSERT_FALSE(base::FilePath::CompareEqualIgnoreCase(expect.value(),
                                                      short_expect));
  EXPECT_TRUE(InstallUtil::ProgramCompare(expect).Evaluate(
      L"\"" + short_expect + L"\""));
}

TEST_F(InstallUtilTest, AddDowngradeVersion) {
  TestBrowserDistribution dist;
  bool system_install = true;
  RegKey(HKEY_LOCAL_MACHINE, dist.GetStateKey().c_str(),
         KEY_SET_VALUE | KEY_WOW64_32KEY);
  std::unique_ptr<WorkItemList> list;

  base::Version current_version("1.1.1.1");
  base::Version higer_new_version("1.1.1.2");
  base::Version lower_new_version_1("1.1.1.0");
  base::Version lower_new_version_2("1.1.0.0");

  ASSERT_FALSE(
      InstallUtil::GetDowngradeVersion(system_install, &dist).IsValid());

  // Upgrade should not create the value.
  list.reset(WorkItem::CreateWorkItemList());
  InstallUtil::AddUpdateDowngradeVersionItem(
      system_install, &current_version, higer_new_version, &dist, list.get());
  ASSERT_TRUE(list->Do());
  ASSERT_FALSE(
      InstallUtil::GetDowngradeVersion(system_install, &dist).IsValid());

  // Downgrade should create the value.
  list.reset(WorkItem::CreateWorkItemList());
  InstallUtil::AddUpdateDowngradeVersionItem(
      system_install, &current_version, lower_new_version_1, &dist, list.get());
  ASSERT_TRUE(list->Do());
  EXPECT_EQ(current_version,
            InstallUtil::GetDowngradeVersion(system_install, &dist));

  // Multiple downgrades should not change the value.
  list.reset(WorkItem::CreateWorkItemList());
  InstallUtil::AddUpdateDowngradeVersionItem(
      system_install, &lower_new_version_1, lower_new_version_2, &dist,
      list.get());
  ASSERT_TRUE(list->Do());
  EXPECT_EQ(current_version,
            InstallUtil::GetDowngradeVersion(system_install, &dist));
}

TEST_F(InstallUtilTest, DeleteDowngradeVersion) {
  TestBrowserDistribution dist;
  bool system_install = true;
  RegKey(HKEY_LOCAL_MACHINE, dist.GetStateKey().c_str(),
         KEY_SET_VALUE | KEY_WOW64_32KEY);
  std::unique_ptr<WorkItemList> list;

  base::Version current_version("1.1.1.1");
  base::Version higer_new_version("1.1.1.2");
  base::Version lower_new_version_1("1.1.1.0");
  base::Version lower_new_version_2("1.1.0.0");

  list.reset(WorkItem::CreateWorkItemList());
  InstallUtil::AddUpdateDowngradeVersionItem(
      system_install, &current_version, lower_new_version_2, &dist, list.get());
  ASSERT_TRUE(list->Do());
  EXPECT_EQ(current_version,
            InstallUtil::GetDowngradeVersion(system_install, &dist));

  // Upgrade should not delete the value if it still lower than the version that
  // downgrade from.
  list.reset(WorkItem::CreateWorkItemList());
  InstallUtil::AddUpdateDowngradeVersionItem(
      system_install, &lower_new_version_2, lower_new_version_1, &dist,
      list.get());
  ASSERT_TRUE(list->Do());
  EXPECT_EQ(current_version,
            InstallUtil::GetDowngradeVersion(system_install, &dist));

  // Repair should not delete the value.
  list.reset(WorkItem::CreateWorkItemList());
  InstallUtil::AddUpdateDowngradeVersionItem(
      system_install, &lower_new_version_1, lower_new_version_1, &dist,
      list.get());
  ASSERT_TRUE(list->Do());
  EXPECT_EQ(current_version,
            InstallUtil::GetDowngradeVersion(system_install, &dist));

  // Fully upgrade should delete the value.
  list.reset(WorkItem::CreateWorkItemList());
  InstallUtil::AddUpdateDowngradeVersionItem(
      system_install, &lower_new_version_1, higer_new_version, &dist,
      list.get());
  ASSERT_TRUE(list->Do());
  ASSERT_FALSE(
      InstallUtil::GetDowngradeVersion(system_install, &dist).IsValid());

  // Fresh install should delete the value if it exists.
  list.reset(WorkItem::CreateWorkItemList());
  InstallUtil::AddUpdateDowngradeVersionItem(
      system_install, &current_version, lower_new_version_2, &dist, list.get());
  ASSERT_TRUE(list->Do());
  EXPECT_EQ(current_version,
            InstallUtil::GetDowngradeVersion(system_install, &dist));
  list.reset(WorkItem::CreateWorkItemList());
  InstallUtil::AddUpdateDowngradeVersionItem(
      system_install, nullptr, lower_new_version_1, &dist, list.get());
  ASSERT_TRUE(list->Do());
  ASSERT_FALSE(
      InstallUtil::GetDowngradeVersion(system_install, &dist).IsValid());
}

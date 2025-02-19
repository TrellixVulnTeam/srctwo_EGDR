// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/arc/arc_util.h"

#include <memory>
#include <string>

#include "ash/public/cpp/app_types.h"
#include "base/command_line.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/test/scoped_feature_list.h"
#include "components/signin/core/account_id/account_id.h"
#include "components/user_manager/fake_user_manager.h"
#include "components/user_manager/user.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/aura/client/aura_constants.h"
#include "ui/aura/test/test_windows.h"
#include "ui/aura/window.h"

namespace arc {
namespace {

// If an instance is created, based on the value passed to the constructor,
// EnableARC feature is enabled/disabled in the scope.
class ScopedArcFeature {
 public:
  explicit ScopedArcFeature(bool enabled) {
    constexpr char kArcFeatureName[] = "EnableARC";
    if (enabled) {
      feature_list.InitFromCommandLine(kArcFeatureName, std::string());
    } else {
      feature_list.InitFromCommandLine(std::string(), kArcFeatureName);
    }
  }
  ~ScopedArcFeature() = default;

 private:
  base::test::ScopedFeatureList feature_list;
  DISALLOW_COPY_AND_ASSIGN(ScopedArcFeature);
};

// Helper to enable user_manager::FakeUserManager while it is in scope.
// TODO(xiyuan): Remove after ScopedUserManagerEnabler is moved to user_manager.
class ScopedUserManager {
 public:
  explicit ScopedUserManager(user_manager::UserManager* user_manager)
      : user_manager_(user_manager) {
    DCHECK(!user_manager::UserManager::IsInitialized());
    user_manager->Initialize();
  }
  ~ScopedUserManager() {
    DCHECK_EQ(user_manager::UserManager::Get(), user_manager_);
    user_manager_->Shutdown();
    user_manager_->Destroy();
  }

 private:
  user_manager::UserManager* const user_manager_;

  DISALLOW_COPY_AND_ASSIGN(ScopedUserManager);
};

// Fake user that can be created with a specified type.
class FakeUser : public user_manager::User {
 public:
  explicit FakeUser(user_manager::UserType user_type)
      : User(AccountId::FromUserEmail("user@test.com")),
        user_type_(user_type) {}
  ~FakeUser() override = default;

  // user_manager::User:
  user_manager::UserType GetType() const override { return user_type_; }

 private:
  const user_manager::UserType user_type_;

  DISALLOW_COPY_AND_ASSIGN(FakeUser);
};

using ArcUtilTest = testing::Test;

TEST_F(ArcUtilTest, IsArcAvailable_None) {
  auto* command_line = base::CommandLine::ForCurrentProcess();

  command_line->InitFromArgv({"", "--arc-availability=none"});
  EXPECT_FALSE(IsArcAvailable());

  // If --arc-availability flag is set to "none", even if Finch experiment is
  // turned on, ARC cannot be used.
  {
    ScopedArcFeature feature(true);
    EXPECT_FALSE(IsArcAvailable());
  }
}

// Test --arc-available with EnableARC feature combination.
TEST_F(ArcUtilTest, IsArcAvailable_Installed) {
  auto* command_line = base::CommandLine::ForCurrentProcess();

  // If ARC is not installed, IsArcAvailable() should return false,
  // regardless of EnableARC feature.
  command_line->InitFromArgv({""});

  // Not available, by-default.
  EXPECT_FALSE(IsArcAvailable());
  EXPECT_FALSE(IsArcKioskAvailable());

  {
    ScopedArcFeature feature(true);
    EXPECT_FALSE(IsArcAvailable());
    EXPECT_FALSE(IsArcKioskAvailable());
  }
  {
    ScopedArcFeature feature(false);
    EXPECT_FALSE(IsArcAvailable());
    EXPECT_FALSE(IsArcKioskAvailable());
  }

  // If ARC is installed, IsArcAvailable() should return true when EnableARC
  // feature is set.
  command_line->InitFromArgv({"", "--arc-available"});

  // Not available, by-default, too.
  EXPECT_FALSE(IsArcAvailable());

  // ARC is available in kiosk mode if installed.
  EXPECT_TRUE(IsArcKioskAvailable());

  {
    ScopedArcFeature feature(true);
    EXPECT_TRUE(IsArcAvailable());
    EXPECT_TRUE(IsArcKioskAvailable());
  }
  {
    ScopedArcFeature feature(false);
    EXPECT_FALSE(IsArcAvailable());
    EXPECT_TRUE(IsArcKioskAvailable());
  }

  // If ARC is installed, IsArcAvailable() should return true when EnableARC
  // feature is set.
  command_line->InitFromArgv({"", "--arc-availability=installed"});

  // Not available, by-default, too.
  EXPECT_FALSE(IsArcAvailable());

  // ARC is available in kiosk mode if installed.
  EXPECT_TRUE(IsArcKioskAvailable());

  {
    ScopedArcFeature feature(true);
    EXPECT_TRUE(IsArcAvailable());
    EXPECT_TRUE(IsArcKioskAvailable());
  }
  {
    ScopedArcFeature feature(false);
    EXPECT_FALSE(IsArcAvailable());
    EXPECT_TRUE(IsArcKioskAvailable());
  }
}

TEST_F(ArcUtilTest, IsArcAvailable_OfficiallySupported) {
  // Regardless of FeatureList, IsArcAvailable() should return true.
  auto* command_line = base::CommandLine::ForCurrentProcess();
  command_line->InitFromArgv({"", "--enable-arc"});
  EXPECT_TRUE(IsArcAvailable());
  EXPECT_TRUE(IsArcKioskAvailable());

  command_line->InitFromArgv({"", "--arc-availability=officially-supported"});
  EXPECT_TRUE(IsArcAvailable());
  EXPECT_TRUE(IsArcKioskAvailable());
}

// TODO(hidehiko): Add test for IsArcKioskMode().
// It depends on UserManager, but a utility to inject fake instance is
// available only in chrome/. To use it in components/, refactoring is needed.

TEST_F(ArcUtilTest, IsArcOptInVerificationDisabled) {
  auto* command_line = base::CommandLine::ForCurrentProcess();
  command_line->InitFromArgv({""});
  EXPECT_FALSE(IsArcOptInVerificationDisabled());

  command_line->InitFromArgv({"", "--disable-arc-opt-in-verification"});
  EXPECT_TRUE(IsArcOptInVerificationDisabled());
}

TEST_F(ArcUtilTest, IsArcAppWindow) {
  std::unique_ptr<aura::Window> window(
      aura::test::CreateTestWindowWithId(0, nullptr));
  EXPECT_FALSE(IsArcAppWindow(window.get()));

  window->SetProperty(aura::client::kAppType,
                      static_cast<int>(ash::AppType::CHROME_APP));
  EXPECT_FALSE(IsArcAppWindow(window.get()));
  window->SetProperty(aura::client::kAppType,
                      static_cast<int>(ash::AppType::ARC_APP));
  EXPECT_TRUE(IsArcAppWindow(window.get()));

  EXPECT_FALSE(IsArcAppWindow(nullptr));
}

TEST_F(ArcUtilTest, IsArcAllowedForUser) {
  user_manager::FakeUserManager fake_user_manager;
  ScopedUserManager scoped_user_manager(&fake_user_manager);

  struct {
    user_manager::UserType user_type;
    bool expected_allowed;
  } const kTestCases[] = {
      {user_manager::USER_TYPE_REGULAR, true},
      {user_manager::USER_TYPE_GUEST, false},
      {user_manager::USER_TYPE_PUBLIC_ACCOUNT, false},
      {user_manager::USER_TYPE_SUPERVISED, false},
      {user_manager::USER_TYPE_KIOSK_APP, false},
      {user_manager::USER_TYPE_CHILD, true},
      {user_manager::USER_TYPE_ARC_KIOSK_APP, true},
      {user_manager::USER_TYPE_ACTIVE_DIRECTORY, true},
  };
  for (const auto& test_case : kTestCases) {
    const FakeUser user(test_case.user_type);
    EXPECT_EQ(test_case.expected_allowed, IsArcAllowedForUser(&user))
        << "User type=" << test_case.user_type;
  }

  // An ephemeral user is a logged in user but unknown to UserManager when
  // ephemeral policy is set.
  fake_user_manager.SetEphemeralUsersEnabled(true);
  fake_user_manager.UserLoggedIn(AccountId::FromUserEmail("test@test.com"),
                                 "test@test.com-hash", false);
  const user_manager::User* ephemeral_user = fake_user_manager.GetActiveUser();
  ASSERT_TRUE(ephemeral_user);
  ASSERT_TRUE(fake_user_manager.IsUserCryptohomeDataEphemeral(
      ephemeral_user->GetAccountId()));

  // Ephemeral user is not allowed for ARC.
  EXPECT_FALSE(IsArcAllowedForUser(ephemeral_user));
}

TEST_F(ArcUtilTest, ArcStartModeDefault) {
  auto* command_line = base::CommandLine::ForCurrentProcess();
  command_line->InitFromArgv({"", "--arc-availability=installed"});
  EXPECT_FALSE(ShouldArcAlwaysStart());
  EXPECT_TRUE(IsPlayStoreAvailable());
}

TEST_F(ArcUtilTest, ArcStartModeWithoutPlayStore) {
  auto* command_line = base::CommandLine::ForCurrentProcess();
  command_line->InitFromArgv(
      {"", "--arc-availability=installed",
       "--arc-start-mode=always-start-with-no-play-store"});
  EXPECT_TRUE(ShouldArcAlwaysStart());
  EXPECT_FALSE(IsPlayStoreAvailable());
}

}  // namespace
}  // namespace arc

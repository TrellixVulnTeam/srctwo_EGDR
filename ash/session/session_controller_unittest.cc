// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/session/session_controller.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ash/login_status.h"
#include "ash/public/interfaces/session_controller.mojom.h"
#include "ash/session/session_controller.h"
#include "ash/session/session_observer.h"
#include "ash/session/test_session_controller_client.h"
#include "ash/shell.h"
#include "ash/test/ash_test_base.h"
#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "components/prefs/testing_pref_service.h"
#include "components/user_manager/user_type.h"
#include "testing/gtest/include/gtest/gtest.h"

using session_manager::SessionState;

namespace ash {
namespace {

class TestSessionObserver : public SessionObserver {
 public:
  TestSessionObserver() : active_account_id_(EmptyAccountId()) {}
  ~TestSessionObserver() override {}

  // SessionObserver:
  void OnActiveUserSessionChanged(const AccountId& account_id) override {
    active_account_id_ = account_id;
  }

  void OnUserSessionAdded(const AccountId& account_id) override {
    user_session_account_ids_.push_back(account_id);
  }

  void OnSessionStateChanged(SessionState state) override { state_ = state; }

  void OnActiveUserPrefServiceChanged(PrefService* pref_service) override {
    last_user_pref_service_ = pref_service;
  }

  std::string GetUserSessionEmails() const {
    std::string emails;
    for (const auto& account_id : user_session_account_ids_) {
      emails += account_id.GetUserEmail() + ",";
    }
    return emails;
  }

  SessionState state() const { return state_; }
  const AccountId& active_account_id() const { return active_account_id_; }
  const std::vector<AccountId>& user_session_account_ids() const {
    return user_session_account_ids_;
  }
  PrefService* last_user_pref_service() const {
    return last_user_pref_service_;
  }
  void clear_last_user_pref_service() { last_user_pref_service_ = nullptr; }

 private:
  SessionState state_ = SessionState::UNKNOWN;
  AccountId active_account_id_;
  std::vector<AccountId> user_session_account_ids_;
  PrefService* last_user_pref_service_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(TestSessionObserver);
};

void FillDefaultSessionInfo(mojom::SessionInfo* info) {
  info->can_lock_screen = true;
  info->should_lock_screen_automatically = true;
  info->is_running_in_app_mode = false;
  info->add_user_session_policy = AddUserSessionPolicy::ALLOWED;
  info->state = SessionState::LOGIN_PRIMARY;
}

class SessionControllerTest : public testing::Test {
 public:
  SessionControllerTest() {}
  ~SessionControllerTest() override {}

  // testing::Test:
  void SetUp() override {
    controller_ = base::MakeUnique<SessionController>(nullptr);
    controller_->AddObserver(&observer_);
  }

  void TearDown() override { controller_->RemoveObserver(&observer_); }

  void SetSessionInfo(const mojom::SessionInfo& info) {
    mojom::SessionInfoPtr info_ptr = mojom::SessionInfo::New();
    *info_ptr = info;
    controller_->SetSessionInfo(std::move(info_ptr));
  }

  void UpdateSession(uint32_t session_id, const std::string& email) {
    mojom::UserSessionPtr session = mojom::UserSession::New();
    session->session_id = session_id;
    session->user_info = mojom::UserInfo::New();
    session->user_info->type = user_manager::USER_TYPE_REGULAR;
    session->user_info->account_id = AccountId::FromUserEmail(email);
    session->user_info->display_name = email;
    session->user_info->display_email = email;
    session->user_info->is_new_profile = false;

    controller_->UpdateUserSession(std::move(session));
  }

  std::string GetUserSessionEmails() const {
    std::string emails;
    for (const auto& session : controller_->GetUserSessions()) {
      emails += session->user_info->display_email + ",";
    }
    return emails;
  }

  SessionController* controller() { return controller_.get(); }
  const TestSessionObserver* observer() const { return &observer_; }

 private:
  std::unique_ptr<SessionController> controller_;
  TestSessionObserver observer_;

  DISALLOW_COPY_AND_ASSIGN(SessionControllerTest);
};

// Tests that the simple session info is reflected properly.
TEST_F(SessionControllerTest, SimpleSessionInfo) {
  mojom::SessionInfo info;
  FillDefaultSessionInfo(&info);
  SetSessionInfo(info);
  UpdateSession(1u, "user1@test.com");

  EXPECT_TRUE(controller()->CanLockScreen());
  EXPECT_TRUE(controller()->ShouldLockScreenAutomatically());
  EXPECT_FALSE(controller()->IsRunningInAppMode());

  info.can_lock_screen = false;
  SetSessionInfo(info);
  EXPECT_FALSE(controller()->CanLockScreen());
  EXPECT_TRUE(controller()->ShouldLockScreenAutomatically());
  EXPECT_FALSE(controller()->IsRunningInAppMode());

  info.should_lock_screen_automatically = false;
  SetSessionInfo(info);
  EXPECT_FALSE(controller()->CanLockScreen());
  EXPECT_FALSE(controller()->ShouldLockScreenAutomatically());
  EXPECT_FALSE(controller()->IsRunningInAppMode());

  info.is_running_in_app_mode = true;
  SetSessionInfo(info);
  EXPECT_FALSE(controller()->CanLockScreen());
  EXPECT_FALSE(controller()->ShouldLockScreenAutomatically());
  EXPECT_TRUE(controller()->IsRunningInAppMode());
}

// Tests that the CanLockScreen is only true with an active user session.
TEST_F(SessionControllerTest, CanLockScreen) {
  mojom::SessionInfo info;
  FillDefaultSessionInfo(&info);
  ASSERT_TRUE(info.can_lock_screen);  // Check can_lock_screen default to true.
  SetSessionInfo(info);

  // Cannot lock screen when there is no active user session.
  EXPECT_FALSE(controller()->IsActiveUserSessionStarted());
  EXPECT_FALSE(controller()->CanLockScreen());

  UpdateSession(1u, "user1@test.com");
  EXPECT_TRUE(controller()->IsActiveUserSessionStarted());
  EXPECT_TRUE(controller()->CanLockScreen());
}

// Tests that AddUserSessionPolicy is set properly.
TEST_F(SessionControllerTest, AddUserPolicy) {
  const AddUserSessionPolicy kTestCases[] = {
      AddUserSessionPolicy::ALLOWED,
      AddUserSessionPolicy::ERROR_NOT_ALLOWED_PRIMARY_USER,
      AddUserSessionPolicy::ERROR_NO_ELIGIBLE_USERS,
      AddUserSessionPolicy::ERROR_MAXIMUM_USERS_REACHED,
  };

  mojom::SessionInfo info;
  FillDefaultSessionInfo(&info);
  for (const auto& policy : kTestCases) {
    info.add_user_session_policy = policy;
    SetSessionInfo(info);
    EXPECT_EQ(policy, controller()->GetAddUserPolicy())
        << "Test case policy=" << static_cast<int>(policy);
  }
}

// Tests that session state can be set and reflected properly.
TEST_F(SessionControllerTest, SessionState) {
  const struct {
    SessionState state;
    bool expected_is_screen_locked;
    bool expected_is_user_session_blocked;
  } kTestCases[] = {
      {SessionState::OOBE, false, true},
      {SessionState::LOGIN_PRIMARY, false, true},
      {SessionState::LOGGED_IN_NOT_ACTIVE, false, false},
      {SessionState::ACTIVE, false, false},
      {SessionState::LOCKED, true, true},
      {SessionState::LOGIN_SECONDARY, false, true},
  };

  mojom::SessionInfo info;
  FillDefaultSessionInfo(&info);
  for (const auto& test_case : kTestCases) {
    info.state = test_case.state;
    SetSessionInfo(info);

    EXPECT_EQ(test_case.state, controller()->GetSessionState())
        << "Test case state=" << static_cast<int>(test_case.state);
    EXPECT_EQ(observer()->state(), controller()->GetSessionState())
        << "Test case state=" << static_cast<int>(test_case.state);
    EXPECT_EQ(test_case.expected_is_screen_locked,
              controller()->IsScreenLocked())
        << "Test case state=" << static_cast<int>(test_case.state);
    EXPECT_EQ(test_case.expected_is_user_session_blocked,
              controller()->IsUserSessionBlocked())
        << "Test case state=" << static_cast<int>(test_case.state);
  }
}

// Tests that LoginStatus is computed correctly for most session states.
TEST_F(SessionControllerTest, GetLoginStatus) {
  const struct {
    SessionState state;
    LoginStatus expected_status;
  } kTestCases[] = {
      {SessionState::UNKNOWN, LoginStatus::NOT_LOGGED_IN},
      {SessionState::OOBE, LoginStatus::NOT_LOGGED_IN},
      {SessionState::LOGIN_PRIMARY, LoginStatus::NOT_LOGGED_IN},
      {SessionState::LOGGED_IN_NOT_ACTIVE, LoginStatus::NOT_LOGGED_IN},
      {SessionState::LOCKED, LoginStatus::LOCKED},
      // TODO: Add LOGIN_SECONDARY if we added a status for it.
  };

  mojom::SessionInfo info;
  FillDefaultSessionInfo(&info);
  for (const auto& test_case : kTestCases) {
    info.state = test_case.state;
    SetSessionInfo(info);
    EXPECT_EQ(test_case.expected_status, controller()->login_status())
        << "Test case state=" << static_cast<int>(test_case.state);
  }
}

// Tests that LoginStatus is computed correctly for active sessions.
TEST_F(SessionControllerTest, GetLoginStateForActiveSession) {
  // Simulate an active user session.
  mojom::SessionInfo info;
  FillDefaultSessionInfo(&info);
  info.state = SessionState::ACTIVE;
  SetSessionInfo(info);

  const struct {
    user_manager::UserType user_type;
    LoginStatus expected_status;
  } kTestCases[] = {
      {user_manager::USER_TYPE_REGULAR, LoginStatus::USER},
      {user_manager::USER_TYPE_GUEST, LoginStatus::GUEST},
      {user_manager::USER_TYPE_PUBLIC_ACCOUNT, LoginStatus::PUBLIC},
      {user_manager::USER_TYPE_SUPERVISED, LoginStatus::SUPERVISED},
      {user_manager::USER_TYPE_KIOSK_APP, LoginStatus::KIOSK_APP},
      {user_manager::USER_TYPE_CHILD, LoginStatus::SUPERVISED},
      {user_manager::USER_TYPE_ARC_KIOSK_APP, LoginStatus::ARC_KIOSK_APP},
      // TODO: Add USER_TYPE_ACTIVE_DIRECTORY if we add a status for it.
  };

  for (const auto& test_case : kTestCases) {
    mojom::UserSessionPtr session = mojom::UserSession::New();
    session->session_id = 1u;
    session->user_info = mojom::UserInfo::New();
    session->user_info->type = test_case.user_type;
    session->user_info->account_id = AccountId::FromUserEmail("user1@test.com");
    session->user_info->display_name = "User 1";
    session->user_info->display_email = "user1@test.com";
    controller()->UpdateUserSession(std::move(session));

    EXPECT_EQ(test_case.expected_status, controller()->login_status())
        << "Test case user_type=" << static_cast<int>(test_case.user_type);
  }
}

// Tests that user sessions can be set and updated.
TEST_F(SessionControllerTest, UserSessions) {
  EXPECT_FALSE(controller()->IsActiveUserSessionStarted());

  UpdateSession(1u, "user1@test.com");
  EXPECT_TRUE(controller()->IsActiveUserSessionStarted());
  EXPECT_EQ("user1@test.com,", GetUserSessionEmails());
  EXPECT_EQ(GetUserSessionEmails(), observer()->GetUserSessionEmails());
  EXPECT_EQ("user1@test.com",
            controller()->GetPrimaryUserSession()->user_info->display_email);

  UpdateSession(2u, "user2@test.com");
  EXPECT_TRUE(controller()->IsActiveUserSessionStarted());
  EXPECT_EQ("user1@test.com,user2@test.com,", GetUserSessionEmails());
  EXPECT_EQ(GetUserSessionEmails(), observer()->GetUserSessionEmails());
  EXPECT_EQ("user1@test.com",
            controller()->GetPrimaryUserSession()->user_info->display_email);

  UpdateSession(1u, "user1_changed@test.com");
  EXPECT_EQ("user1_changed@test.com,user2@test.com,", GetUserSessionEmails());
  // TODO(xiyuan): Verify observer gets the updated user session info.
}

// Tests that user sessions can be ordered.
TEST_F(SessionControllerTest, ActiveSession) {
  UpdateSession(1u, "user1@test.com");
  UpdateSession(2u, "user2@test.com");
  EXPECT_EQ("user1@test.com",
            controller()->GetPrimaryUserSession()->user_info->display_email);

  std::vector<uint32_t> order = {1u, 2u};
  controller()->SetUserSessionOrder(order);
  EXPECT_EQ("user1@test.com,user2@test.com,", GetUserSessionEmails());
  EXPECT_EQ("user1@test.com", observer()->active_account_id().GetUserEmail());
  EXPECT_EQ("user1@test.com",
            controller()->GetPrimaryUserSession()->user_info->display_email);

  order = {2u, 1u};
  controller()->SetUserSessionOrder(order);
  EXPECT_EQ("user2@test.com,user1@test.com,", GetUserSessionEmails());
  EXPECT_EQ("user2@test.com", observer()->active_account_id().GetUserEmail());
  EXPECT_EQ("user1@test.com",
            controller()->GetPrimaryUserSession()->user_info->display_email);

  order = {1u, 2u};
  controller()->SetUserSessionOrder(order);
  EXPECT_EQ("user1@test.com,user2@test.com,", GetUserSessionEmails());
  EXPECT_EQ("user1@test.com", observer()->active_account_id().GetUserEmail());
  EXPECT_EQ("user1@test.com",
            controller()->GetPrimaryUserSession()->user_info->display_email);
}

// Tests that user session is unblocked with a running unlock animation so that
// focus rules can find a correct activatable window after screen lock is
// dismissed.
TEST_F(SessionControllerTest, UserSessionUnblockedWithRunningUnlockAnimation) {
  mojom::SessionInfo info;
  FillDefaultSessionInfo(&info);

  // LOCKED means blocked user session.
  info.state = SessionState::LOCKED;
  SetSessionInfo(info);
  EXPECT_TRUE(controller()->IsUserSessionBlocked());

  // Mark a running unlock animation unblocks user session.
  controller()->RunUnlockAnimation(base::Closure());
  EXPECT_FALSE(controller()->IsUserSessionBlocked());

  const struct {
    SessionState state;
    bool expected_is_user_session_blocked;
  } kTestCases[] = {
      {SessionState::OOBE, true},
      {SessionState::LOGIN_PRIMARY, true},
      {SessionState::LOGGED_IN_NOT_ACTIVE, false},
      {SessionState::ACTIVE, false},
      {SessionState::LOGIN_SECONDARY, true},
  };
  for (const auto& test_case : kTestCases) {
    info.state = test_case.state;
    SetSessionInfo(info);

    // Mark a running unlock animation.
    controller()->RunUnlockAnimation(base::Closure());

    EXPECT_EQ(test_case.expected_is_user_session_blocked,
              controller()->IsUserSessionBlocked())
        << "Test case state=" << static_cast<int>(test_case.state);
  }
}

TEST_F(SessionControllerTest, IsUserSupervised) {
  mojom::UserSessionPtr session = mojom::UserSession::New();
  session->session_id = 1u;
  session->user_info = mojom::UserInfo::New();
  session->user_info->type = user_manager::USER_TYPE_SUPERVISED;
  controller()->UpdateUserSession(std::move(session));

  EXPECT_TRUE(controller()->IsUserSupervised());
}

TEST_F(SessionControllerTest, IsUserChild) {
  mojom::UserSessionPtr session = mojom::UserSession::New();
  session->session_id = 1u;
  session->user_info = mojom::UserInfo::New();
  session->user_info->type = user_manager::USER_TYPE_CHILD;
  controller()->UpdateUserSession(std::move(session));

  EXPECT_TRUE(controller()->IsUserChild());

  // Child accounts are supervised.
  EXPECT_TRUE(controller()->IsUserSupervised());
}

using SessionControllerPrefsTest = NoSessionAshTestBase;

// Verifies that ShellObserver is notified for PrefService changes.
TEST_F(SessionControllerPrefsTest, Observer) {
  constexpr char kUser1[] = "user1@test.com";
  constexpr char kUser2[] = "user2@test.com";
  const AccountId kUserAccount1 = AccountId::FromUserEmail(kUser1);
  const AccountId kUserAccount2 = AccountId::FromUserEmail(kUser2);

  TestSessionObserver observer;
  SessionController* controller = Shell::Get()->session_controller();
  controller->AddObserver(&observer);

  // Setup 2 users.
  TestSessionControllerClient* session = GetSessionControllerClient();
  // Disable auto-provision of PrefService for each user.
  constexpr bool kEnableSettings = true;
  constexpr bool kProvidePrefService = false;
  session->AddUserSession(kUser1, user_manager::USER_TYPE_REGULAR,
                          kEnableSettings, kProvidePrefService);
  session->AddUserSession(kUser2, user_manager::USER_TYPE_REGULAR,
                          kEnableSettings, kProvidePrefService);

  // The observer is not notified because the PrefService for kUser1 is not yet
  // ready.
  session->SwitchActiveUser(kUserAccount1);
  EXPECT_EQ(nullptr, observer.last_user_pref_service());

  auto pref_service = base::MakeUnique<TestingPrefServiceSimple>();
  Shell::RegisterProfilePrefs(pref_service->registry(), true /* for_test */);
  controller->ProvideUserPrefServiceForTest(kUserAccount1,
                                            std::move(pref_service));
  EXPECT_EQ(controller->GetUserPrefServiceForUser(kUserAccount1),
            observer.last_user_pref_service());
  EXPECT_EQ(controller->GetUserPrefServiceForUser(kUserAccount1),
            controller->GetLastActiveUserPrefService());

  observer.clear_last_user_pref_service();

  // Switching to a user for which prefs are not ready does not notify and
  // GetLastActiveUserPrefService() returns the old PrefService.
  session->SwitchActiveUser(AccountId::FromUserEmail(kUser2));
  EXPECT_EQ(nullptr, observer.last_user_pref_service());
  EXPECT_EQ(controller->GetUserPrefServiceForUser(kUserAccount1),
            controller->GetLastActiveUserPrefService());

  session->SwitchActiveUser(AccountId::FromUserEmail(kUser1));
  EXPECT_EQ(controller->GetUserPrefServiceForUser(kUserAccount1),
            observer.last_user_pref_service());
  EXPECT_EQ(controller->GetUserPrefServiceForUser(kUserAccount1),
            controller->GetLastActiveUserPrefService());

  // There should be no notification about a PrefService for an inactive user
  // becoming initialized.
  observer.clear_last_user_pref_service();
  pref_service = base::MakeUnique<TestingPrefServiceSimple>();
  Shell::RegisterProfilePrefs(pref_service->registry(), true /* for_test */);
  controller->ProvideUserPrefServiceForTest(kUserAccount2,
                                            std::move(pref_service));
  EXPECT_EQ(nullptr, observer.last_user_pref_service());

  session->SwitchActiveUser(AccountId::FromUserEmail(kUser2));
  EXPECT_EQ(controller->GetUserPrefServiceForUser(kUserAccount2),
            observer.last_user_pref_service());
  EXPECT_EQ(controller->GetUserPrefServiceForUser(kUserAccount2),
            controller->GetLastActiveUserPrefService());

  controller->RemoveObserver(&observer);
}

TEST_F(SessionControllerTest, GetUserType) {
  // Child accounts
  mojom::UserSessionPtr session = mojom::UserSession::New();
  session->session_id = 1u;
  session->user_info = mojom::UserInfo::New();
  session->user_info->type = user_manager::USER_TYPE_CHILD;
  controller()->UpdateUserSession(std::move(session));
  EXPECT_EQ(user_manager::USER_TYPE_CHILD, controller()->GetUserType());

  // Regular accounts
  session = mojom::UserSession::New();
  session->session_id = 1u;
  session->user_info = mojom::UserInfo::New();
  session->user_info->type = user_manager::USER_TYPE_REGULAR;
  controller()->UpdateUserSession(std::move(session));
  EXPECT_EQ(user_manager::USER_TYPE_REGULAR, controller()->GetUserType());
}

TEST_F(SessionControllerTest, IsUserPrimary) {
  controller()->ClearUserSessionsForTest();

  // The first added user is a primary user
  mojom::UserSessionPtr session = mojom::UserSession::New();
  session->session_id = 1u;
  session->user_info = mojom::UserInfo::New();
  session->user_info->type = user_manager::USER_TYPE_REGULAR;
  controller()->UpdateUserSession(std::move(session));
  EXPECT_TRUE(controller()->IsUserPrimary());

  // The users added thereafter are not primary users
  session = mojom::UserSession::New();
  session->session_id = 2u;
  session->user_info = mojom::UserInfo::New();
  session->user_info->type = user_manager::USER_TYPE_REGULAR;
  controller()->UpdateUserSession(std::move(session));
  // Simulates user switching by changing the order of session_ids.
  controller()->SetUserSessionOrder({2u, 1u});
  EXPECT_FALSE(controller()->IsUserPrimary());
}

TEST_F(SessionControllerTest, IsUserFirstLogin) {
  mojom::UserSessionPtr session = mojom::UserSession::New();
  session->session_id = 1u;
  session->user_info = mojom::UserInfo::New();
  session->user_info->type = user_manager::USER_TYPE_REGULAR;
  controller()->UpdateUserSession(std::move(session));
  EXPECT_FALSE(controller()->IsUserFirstLogin());

  // user_info->is_new_profile being true means the user is first time login.
  session = mojom::UserSession::New();
  session->session_id = 1u;
  session->user_info = mojom::UserInfo::New();
  session->user_info->type = user_manager::USER_TYPE_REGULAR;
  session->user_info->is_new_profile = true;
  controller()->UpdateUserSession(std::move(session));
  EXPECT_TRUE(controller()->IsUserFirstLogin());
}

}  // namespace
}  // namespace ash

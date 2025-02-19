// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/ash/multi_user/multi_user_util.h"

#include "chrome/browser/chromeos/profiles/profile_helper.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/ash/multi_user/multi_user_window_manager.h"
#include "components/signin/core/account_id/account_id.h"
#include "components/user_manager/user_manager.h"

namespace multi_user_util {

AccountId GetAccountIdFromProfile(Profile* profile) {
  // This will guarantee an nonempty AccountId be returned if a valid profile is
  // provided.
  const user_manager::User* user =
      chromeos::ProfileHelper::Get()->GetUserByProfile(
          profile->GetOriginalProfile());
  return user ? user->GetAccountId() : EmptyAccountId();
}

AccountId GetAccountIdFromEmail(const std::string& email) {
  // |email| and profile name could be empty if not yet logged in or guest mode.
  return email.empty() ? EmptyAccountId()
                       : AccountId::FromUserEmail(gaia::CanonicalizeEmail(
                             gaia::SanitizeEmail(email)));
}

Profile* GetProfileFromAccountId(const AccountId& account_id) {
  const user_manager::User* user =
      user_manager::UserManager::Get()->FindUser(account_id);
  return user ? chromeos::ProfileHelper::Get()->GetProfileByUser(user)
              : nullptr;
}

Profile* GetProfileFromWindow(aura::Window* window) {
  chrome::MultiUserWindowManager* manager =
      chrome::MultiUserWindowManager::GetInstance();
  // We might come here before the manager got created - or in a unit test.
  if (!manager)
    return nullptr;
  const AccountId account_id = manager->GetUserPresentingWindow(window);
  return account_id.is_valid() ? GetProfileFromAccountId(account_id) : nullptr;
}

bool IsProfileFromActiveUser(Profile* profile) {
  // There may be no active user in tests.
  const user_manager::User* active_user =
      user_manager::UserManager::Get()->GetActiveUser();
  if (!active_user)
    return true;
  return GetAccountIdFromProfile(profile) == active_user->GetAccountId();
}

const AccountId GetCurrentAccountId() {
  const user_manager::User* user =
      user_manager::UserManager::Get()->GetActiveUser();
  // In unit tests user login phase is usually skipped.
  return user ? user->GetAccountId() : EmptyAccountId();
}

// Move the window to the current user's desktop.
void MoveWindowToCurrentDesktop(aura::Window* window) {
  if (!chrome::MultiUserWindowManager::GetInstance()->IsWindowOnDesktopOfUser(
          window, GetCurrentAccountId())) {
    chrome::MultiUserWindowManager::GetInstance()->ShowWindowForUser(
        window, GetCurrentAccountId());
  }
}

}  // namespace multi_user_util

// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PROFILES_PROFILE_ATTRIBUTES_ENTRY_H_
#define CHROME_BROWSER_PROFILES_PROFILE_ATTRIBUTES_ENTRY_H_

#include <stddef.h>

#include <memory>
#include <string>

#include "base/files/file_path.h"
#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/time/time.h"
#include "base/values.h"

namespace gfx {
class Image;
}

class PrefService;
class ProfileInfoCache;

class ProfileAttributesEntry {
 public:
  ProfileAttributesEntry();
  virtual ~ProfileAttributesEntry() {}

  // Gets the name of the profile, which is the one displayed in the User Menu.
  base::string16 GetName() const;

  base::string16 GetShortcutName() const;
  // Gets the path to the profile. Should correspond to the path passed to
  // ProfileAttributesStorage::GetProfileAttributesWithPath to get this entry.
  base::FilePath GetPath() const;
  base::Time GetActiveTime() const;
  // Gets the user name of the signed in profile. This is typically the email
  // address used to sign in and the empty string for profiles that aren't
  // signed in to chrome.
  base::string16 GetUserName() const;
  // Gets the icon used as this profile's avatar. This might not be the icon
  // displayed in the UI if IsUsingGAIAPicture() is true.
  const gfx::Image& GetAvatarIcon() const;
  std::string GetLocalAuthCredentials() const;
  std::string GetPasswordChangeDetectionToken() const;
  // Returns true if the profile is currently running any background apps. Note
  // that a return value of false could mean an error in collection or that
  // there are currently no background apps running. However, the action which
  // results is the same in both cases (thus far).
  bool GetBackgroundStatus() const;
  // Gets the GAIA full name associated with this profile if it's signed in.
  base::string16 GetGAIAName() const;
  // Gets the GAIA given name associated with this profile if it's signed in.
  base::string16 GetGAIAGivenName() const;
  // Gets the opaque string representation of the profile's GAIA ID if it's
  // signed in.
  std::string GetGAIAId() const;
  // Returns the GAIA picture for the given profile. This may return NULL
  // if the profile does not have a GAIA picture or if the picture must be
  // loaded from disk.
  const gfx::Image* GetGAIAPicture() const;
  // Returns true if the profile displays a GAIA picture instead of one of the
  // locally bundled icons.
  bool IsUsingGAIAPicture() const;
  // Returns true if a GAIA picture has been loaded or has failed to load.
  bool IsGAIAPictureLoaded() const;
  // Returns true if the profile is signed in as a supervised user.
  bool IsSupervised() const;
  // Returns true if the profile is signed in as a child account.
  bool IsChild() const;
  // Returns true if the profile is a supervised user but not a child account.
  bool IsLegacySupervised() const;
  bool IsOmitted() const;
  bool IsSigninRequired() const;
  // Gets the supervised user ID of the profile's signed in account, if it's a
  // supervised user.
  std::string GetSupervisedUserId() const;
  // Returns true if the profile is an ephemeral profile.
  bool IsEphemeral() const;
  // Returns true if the profile is using a default name, typically of the
  // format "Person %d".
  bool IsUsingDefaultName() const;
  // Returns true if the profile is signed in.
  bool IsAuthenticated() const;
  // Returns true if the Profile is using the default avatar, which is one of
  // the profile icons selectable at profile creation.
  bool IsUsingDefaultAvatar() const;
  // Returns true if the profile is signed in but is in an authentication error
  // state.
  bool IsAuthError() const;
  // Returns the index of the default icon used by the profile.
  size_t GetAvatarIconIndex() const;

  void SetName(const base::string16& name);
  void SetShortcutName(const base::string16& name);
  void SetActiveTimeToNow();
  void SetIsOmitted(bool is_omitted);
  void SetSupervisedUserId(const std::string& id);
  void SetLocalAuthCredentials(const std::string& auth);
  void SetPasswordChangeDetectionToken(const std::string& token);
  void SetBackgroundStatus(bool running_background_apps);
  void SetGAIAName(const base::string16& name);
  void SetGAIAGivenName(const base::string16& name);
  void SetGAIAPicture(const gfx::Image* image);
  void SetIsUsingGAIAPicture(bool value);
  void SetIsSigninRequired(bool value);
  void SetIsEphemeral(bool value);
  void SetIsUsingDefaultName(bool value);
  void SetIsUsingDefaultAvatar(bool value);
  void SetIsAuthError(bool value);
  void SetAvatarIconIndex(size_t icon_index);

  void SetAuthInfo(const std::string& gaia_id, const base::string16& user_name);

  // Lock/Unlock the profile, should be called only if force-sign-in is enabled.
  void LockForceSigninProfile(bool is_lock);

 private:
  friend class ProfileInfoCache;
  FRIEND_TEST_ALL_PREFIXES(ProfileAttributesStorageTest,
                           EntryInternalAccessors);
  FRIEND_TEST_ALL_PREFIXES(ProfileAttributesStorageTest, ProfileActiveTime);

  void Initialize(ProfileInfoCache* cache,
                  const base::FilePath& path,
                  PrefService* prefs);

  // Loads and saves the data to the local state.
  const base::Value* GetEntryData() const;
  void SetEntryData(base::Value data);

  // Internal getter that returns a base::Value*, or nullptr if the key is not
  // present.
  const base::Value* GetValue(const char* key) const;

  // Internal getters that return basic data types. If the key is not present,
  // or if the data is in a wrong data type, return empty string, 0.0, or false
  // depending on the target data type. We do not assume that the data type is
  // correct because the local state file can be modified by a third party.
  std::string GetString(const char* key) const;
  base::string16 GetString16(const char* key) const;
  double GetDouble(const char* key) const;
  bool GetBool(const char* key) const;

  // Type checking. Only IsDouble is implemented because others do not have
  // callsites.
  bool IsDouble(const char* key) const;

  // Internal setters that accept basic data types. Return if the original data
  // is different from the new data, i.e. whether actual update is done.
  bool SetString(const char* key, std::string value);
  bool SetString16(const char* key, base::string16 value);
  bool SetDouble(const char* key, double value);
  bool SetBool(const char* key, bool value);

  // These members are an implementation detail meant to smooth the migration
  // of the ProfileInfoCache to the ProfileAttributesStorage interface. They can
  // be safely removed once the ProfileInfoCache stops using indices
  // internally.
  // TODO(anthonyvd): Remove ProfileInfoCache related implementation details
  // when this class holds the members required to fulfill its own contract.
  size_t profile_index() const;

  ProfileInfoCache* profile_info_cache_;
  PrefService* prefs_;
  base::FilePath profile_path_;
  std::string storage_key_;

  // A separate boolean flag indicates whether the signin is required when force
  // signin is enabled. So that the profile locked status will be stored in
  // memory only and can be easily reset once the policy is turned off.
  bool is_force_signin_profile_locked_ = false;
  bool is_force_signin_enabled_;

  DISALLOW_COPY_AND_ASSIGN(ProfileAttributesEntry);
};

#endif  // CHROME_BROWSER_PROFILES_PROFILE_ATTRIBUTES_ENTRY_H_

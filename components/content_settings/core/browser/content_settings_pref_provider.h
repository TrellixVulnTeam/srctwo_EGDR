// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_CONTENT_SETTINGS_CORE_BROWSER_CONTENT_SETTINGS_PREF_PROVIDER_H_
#define COMPONENTS_CONTENT_SETTINGS_CORE_BROWSER_CONTENT_SETTINGS_PREF_PROVIDER_H_

// A content settings provider that takes its settings out of the pref service.

#include <map>
#include <memory>
#include <vector>

#include "base/macros.h"
#include "components/content_settings/core/browser/content_settings_utils.h"
#include "components/content_settings/core/browser/user_modifiable_provider.h"
#include "components/prefs/pref_change_registrar.h"

class PrefService;

namespace base {
class Clock;
}

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace content_settings {

class ContentSettingsPref;

// Content settings provider that provides content settings from the user
// preference.
class PrefProvider : public UserModifiableProvider {
 public:
  static void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

  PrefProvider(PrefService* prefs, bool incognito, bool store_last_modified);
  ~PrefProvider() override;

  // UserModifiableProvider implementations.
  std::unique_ptr<RuleIterator> GetRuleIterator(
      ContentSettingsType content_type,
      const ResourceIdentifier& resource_identifier,
      bool incognito) const override;
  bool SetWebsiteSetting(const ContentSettingsPattern& primary_pattern,
                         const ContentSettingsPattern& secondary_pattern,
                         ContentSettingsType content_type,
                         const ResourceIdentifier& resource_identifier,
                         base::Value* value) override;
  void ClearAllContentSettingsRules(ContentSettingsType content_type) override;
  void ShutdownOnUIThread() override;
  base::Time GetWebsiteSettingLastModified(
      const ContentSettingsPattern& primary_pattern,
      const ContentSettingsPattern& secondary_pattern,
      ContentSettingsType content_type,
      const ResourceIdentifier& resource_identifier) override;

  void ClearPrefs();

  ContentSettingsPref* GetPref(ContentSettingsType type) const;

  void SetClockForTesting(std::unique_ptr<base::Clock> clock);

 private:
  friend class DeadlockCheckerObserver;  // For testing.

  void Notify(const ContentSettingsPattern& primary_pattern,
              const ContentSettingsPattern& secondary_pattern,
              ContentSettingsType content_type,
              const std::string& resource_identifier);

  // Clean up the obsolete preferences from the user's profile.
  void DiscardObsoletePreferences();

  // Weak; owned by the Profile and reset in ShutdownOnUIThread.
  PrefService* prefs_;

  const bool is_incognito_;

  bool store_last_modified_;

  PrefChangeRegistrar pref_change_registrar_;

  std::map<ContentSettingsType, std::unique_ptr<ContentSettingsPref>>
      content_settings_prefs_;

  base::ThreadChecker thread_checker_;

  std::unique_ptr<base::Clock> clock_;

  DISALLOW_COPY_AND_ASSIGN(PrefProvider);
};

}  // namespace content_settings

#endif  // COMPONENTS_CONTENT_SETTINGS_CORE_BROWSER_CONTENT_SETTINGS_PREF_PROVIDER_H_

// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/content_settings/core/browser/content_settings_info.h"

#include "base/stl_util.h"
#include "components/content_settings/core/browser/website_settings_info.h"

namespace content_settings {

ContentSettingsInfo::ContentSettingsInfo(
    const WebsiteSettingsInfo* website_settings_info,
    const std::vector<std::string>& whitelisted_schemes,
    const std::set<ContentSetting>& valid_settings,
    IncognitoBehavior incognito_behavior)
    : website_settings_info_(website_settings_info),
      whitelisted_schemes_(whitelisted_schemes),
      valid_settings_(valid_settings),
      incognito_behavior_(incognito_behavior) {}

ContentSettingsInfo::~ContentSettingsInfo() {}

bool ContentSettingsInfo::IsSettingValid(ContentSetting setting) const {
  return base::ContainsKey(valid_settings_, setting);
}

// TODO(raymes): Find a better way to deal with the special-casing in
// IsDefaultSettingValid.
bool ContentSettingsInfo::IsDefaultSettingValid(ContentSetting setting) const {
  ContentSettingsType type = website_settings_info_->type();
#if defined(OS_ANDROID) || defined(OS_CHROMEOS)
  // Don't support ALLOW for protected media default setting until migration.
  if (type == CONTENT_SETTINGS_TYPE_PROTECTED_MEDIA_IDENTIFIER &&
      setting == CONTENT_SETTING_ALLOW) {
    return false;
  }
#endif

  // Don't support ALLOW for the default media settings.
  if ((type == CONTENT_SETTINGS_TYPE_MEDIASTREAM_CAMERA ||
       type == CONTENT_SETTINGS_TYPE_MEDIASTREAM_MIC) &&
      setting == CONTENT_SETTING_ALLOW) {
    return false;
  }

  return base::ContainsKey(valid_settings_, setting);
}

}  // namespace content_settings

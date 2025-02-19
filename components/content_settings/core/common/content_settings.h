// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_CONTENT_SETTINGS_CORE_COMMON_CONTENT_SETTINGS_H_
#define COMPONENTS_CONTENT_SETTINGS_CORE_COMMON_CONTENT_SETTINGS_H_

#include <stddef.h>

#include <memory>
#include <string>
#include <vector>

#include "base/memory/ptr_util.h"
#include "base/values.h"
#include "components/content_settings/core/common/content_settings_pattern.h"
#include "components/content_settings/core/common/content_settings_types.h"

// Different settings that can be assigned for a particular content type.  We
// give the user the ability to set these on a global and per-origin basis.
// A Java counterpart will be generated for this enum.
// GENERATED_JAVA_ENUM_PACKAGE: org.chromium.chrome.browser.preferences.website
// GENERATED_JAVA_CLASS_NAME_OVERRIDE: ContentSettingValues
//
// TODO(nigeltao): migrate the Java users of this enum to the mojom-generated
// enum.
enum ContentSetting {
  CONTENT_SETTING_DEFAULT = 0,
  CONTENT_SETTING_ALLOW,
  CONTENT_SETTING_BLOCK,
  CONTENT_SETTING_ASK,
  CONTENT_SETTING_SESSION_ONLY,
  CONTENT_SETTING_DETECT_IMPORTANT_CONTENT,
  CONTENT_SETTING_NUM_SETTINGS
};

// Range-checked conversion of an int to a ContentSetting, for use when reading
// prefs off disk.
ContentSetting IntToContentSetting(int content_setting);

// Converts a given content setting to its histogram value, for use when saving
// content settings types to a histogram.
int ContentSettingTypeToHistogramValue(ContentSettingsType content_setting,
                                       size_t* num_values);

struct ContentSettingPatternSource {
  ContentSettingPatternSource(const ContentSettingsPattern& primary_pattern,
                              const ContentSettingsPattern& secondary_patttern,
                              std::unique_ptr<base::Value> setting_value,
                              const std::string& source,
                              bool incognito);
  ContentSettingPatternSource(const ContentSettingPatternSource& other);
  ContentSettingPatternSource();
  ContentSettingPatternSource& operator=(
      const ContentSettingPatternSource& other);
  ~ContentSettingPatternSource();
  ContentSetting GetContentSetting() const;

  ContentSettingsPattern primary_pattern;
  ContentSettingsPattern secondary_pattern;
  std::unique_ptr<base::Value> setting_value;
  std::string source;
  bool incognito;
};

typedef std::vector<ContentSettingPatternSource> ContentSettingsForOneType;

struct RendererContentSettingRules {
  RendererContentSettingRules();
  ~RendererContentSettingRules();
  ContentSettingsForOneType image_rules;
  ContentSettingsForOneType script_rules;
  ContentSettingsForOneType autoplay_rules;
  ContentSettingsForOneType client_hints_rules;
};

namespace content_settings {

typedef std::string ResourceIdentifier;

// Enum containing the various source for content settings. Settings can be
// set by policy, extension, the user or by the custodian of a supervised user.
// Certain (internal) schemes are whilelisted. For whilelisted schemes the
// source is |SETTING_SOURCE_WHITELIST|.
enum SettingSource {
  SETTING_SOURCE_NONE,
  SETTING_SOURCE_POLICY,
  SETTING_SOURCE_EXTENSION,
  SETTING_SOURCE_USER,
  SETTING_SOURCE_WHITELIST,
  SETTING_SOURCE_SUPERVISED,
};

// |SettingInfo| provides meta data for content setting values. |source|
// contains the source of a value. |primary_pattern| and |secondary_pattern|
// contains the patterns of the appling rule.
struct SettingInfo {
  SettingSource source;
  ContentSettingsPattern primary_pattern;
  ContentSettingsPattern secondary_pattern;
};

}  // namespace content_settings

#endif  // COMPONENTS_CONTENT_SETTINGS_CORE_COMMON_CONTENT_SETTINGS_H_

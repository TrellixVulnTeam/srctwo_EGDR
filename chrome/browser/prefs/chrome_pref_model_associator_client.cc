// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/prefs/chrome_pref_model_associator_client.h"

#include "base/memory/singleton.h"
#include "chrome/common/pref_names.h"
#include "components/content_settings/core/browser/website_settings_info.h"
#include "components/content_settings/core/browser/website_settings_registry.h"

// static
ChromePrefModelAssociatorClient*
ChromePrefModelAssociatorClient::GetInstance() {
  return base::Singleton<ChromePrefModelAssociatorClient>::get();
}

ChromePrefModelAssociatorClient::ChromePrefModelAssociatorClient() {}

ChromePrefModelAssociatorClient::~ChromePrefModelAssociatorClient() {}

bool ChromePrefModelAssociatorClient::IsMergeableListPreference(
    const std::string& pref_name) const {
  return pref_name == prefs::kURLsToRestoreOnStartup;
}

bool ChromePrefModelAssociatorClient::IsMergeableDictionaryPreference(
    const std::string& pref_name) const {
  const content_settings::WebsiteSettingsRegistry& registry =
      *content_settings::WebsiteSettingsRegistry::GetInstance();
  for (const content_settings::WebsiteSettingsInfo* info : registry) {
    if (info->pref_name() == pref_name)
      return true;
  }
  return false;
}

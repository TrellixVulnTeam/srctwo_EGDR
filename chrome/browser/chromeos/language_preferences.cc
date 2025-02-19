// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/language_preferences.h"
#include "components/prefs/pref_registry_simple.h"

namespace chromeos {
namespace language_prefs {

// ---------------------------------------------------------------------------
// For ibus-daemon
// ---------------------------------------------------------------------------
const char kGeneralSectionName[] = "general";
const char kPreloadEnginesConfigName[] = "preload_engines";

// ---------------------------------------------------------------------------
// For keyboard stuff
// ---------------------------------------------------------------------------
const bool kXkbAutoRepeatEnabled = true;
const int kXkbAutoRepeatDelayInMs = 500;
const int kXkbAutoRepeatIntervalInMs = 50;
const char kPreferredKeyboardLayout[] = "PreferredKeyboardLayout";

void RegisterPrefs(PrefRegistrySimple* registry) {
  // We use an empty string here rather than a hardware keyboard layout name
  // since input_method::GetHardwareInputMethodId() might return a fallback
  // layout name if registry->RegisterStringPref(kHardwareKeyboardLayout)
  // is not called yet.
  registry->RegisterStringPref(kPreferredKeyboardLayout, "");
}

}  // namespace language_prefs
}  // namespace chromeos

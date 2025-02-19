// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/memory/ptr_util.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/prefs/chrome_command_line_pref_store.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/pref_names.h"
#include "chrome/test/base/chrome_render_view_host_test_harness.h"
#include "chrome/test/base/testing_profile.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/sync_preferences/testing_pref_service_syncable.h"
#include "content/public/common/web_preferences.h"
#include "content/public/test/test_renderer_host.h"

using content::RenderViewHostTester;
using content::WebPreferences;

TEST(ChromePrefServiceTest, UpdateCommandLinePrefStore) {
  TestingPrefServiceSimple prefs;
  prefs.registry()->RegisterBooleanPref(prefs::kCloudPrintProxyEnabled, false);

  // Check to make sure the value is as expected.
  const PrefService::Preference* pref =
      prefs.FindPreference(prefs::kCloudPrintProxyEnabled);
  ASSERT_TRUE(pref);
  const base::Value* value = pref->GetValue();
  ASSERT_TRUE(value);
  EXPECT_EQ(base::Value::Type::BOOLEAN, value->GetType());
  bool actual_bool_value = true;
  EXPECT_TRUE(value->GetAsBoolean(&actual_bool_value));
  EXPECT_FALSE(actual_bool_value);

  // Change the command line.
  base::CommandLine cmd_line(base::CommandLine::NO_PROGRAM);
  cmd_line.AppendSwitch(switches::kEnableCloudPrintProxy);

  // Call UpdateCommandLinePrefStore and check to see if the value has changed.
  prefs.UpdateCommandLinePrefStore(new ChromeCommandLinePrefStore(&cmd_line));
  pref = prefs.FindPreference(prefs::kCloudPrintProxyEnabled);
  ASSERT_TRUE(pref);
  value = pref->GetValue();
  ASSERT_TRUE(value);
  EXPECT_EQ(base::Value::Type::BOOLEAN, value->GetType());
  actual_bool_value = false;
  EXPECT_TRUE(value->GetAsBoolean(&actual_bool_value));
  EXPECT_TRUE(actual_bool_value);
}

class ChromePrefServiceWebKitPrefs : public ChromeRenderViewHostTestHarness {
 protected:
  void SetUp() override {
    ChromeRenderViewHostTestHarness::SetUp();

    // Supply our own profile so we use the correct profile data. The test
    // harness is not supposed to overwrite a profile if it's already created.

    // Set some (WebKit) user preferences.
    sync_preferences::TestingPrefServiceSyncable* pref_services =
        profile()->GetTestingPrefService();
    pref_services->SetUserPref(prefs::kDefaultCharset,
                               base::MakeUnique<base::Value>("utf8"));
    pref_services->SetUserPref(prefs::kWebKitDefaultFontSize,
                               base::MakeUnique<base::Value>(20));
    pref_services->SetUserPref(prefs::kWebKitTextAreasAreResizable,
                               base::MakeUnique<base::Value>(false));
    pref_services->SetUserPref("webkit.webprefs.foo",
                               base::MakeUnique<base::Value>("bar"));
  }
};

// Tests to see that webkit preferences are properly loaded and copied over
// to a WebPreferences object.
TEST_F(ChromePrefServiceWebKitPrefs, PrefsCopied) {
  WebPreferences webkit_prefs =
      RenderViewHostTester::For(rvh())->TestComputeWebkitPrefs();

  // These values have been overridden by the profile preferences.
  EXPECT_EQ("UTF-8", webkit_prefs.default_encoding);
#if !defined(OS_ANDROID)
  EXPECT_EQ(20, webkit_prefs.default_font_size);
#else
  // This pref is not configurable on Android so the default of 16 is always
  // used.
  EXPECT_EQ(16, webkit_prefs.default_font_size);
#endif
  EXPECT_FALSE(webkit_prefs.text_areas_are_resizable);

  // These should still be the default values.
#if defined(OS_MACOSX)
  const char kDefaultFont[] = "Times";
#elif defined(OS_CHROMEOS)
  const char kDefaultFont[] = "Tinos";
#else
  const char kDefaultFont[] = "Times New Roman";
#endif
  EXPECT_EQ(base::ASCIIToUTF16(kDefaultFont),
            webkit_prefs.standard_font_family_map[prefs::kWebKitCommonScript]);
  EXPECT_TRUE(webkit_prefs.javascript_enabled);
}

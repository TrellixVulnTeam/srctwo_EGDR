// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/public/cpp/ash_pref_names.h"
#include "base/sys_info.h"
#include "base/values.h"
#include "chrome/browser/chromeos/arc/arc_util.h"
#include "chrome/browser/chromeos/policy/browser_policy_connector_chromeos.h"
#include "chrome/browser/chromeos/settings/cros_settings.h"
#include "chrome/browser/chromeos/settings/stub_install_attributes.h"
#include "chrome/browser/extensions/extension_apitest.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/pref_names.h"
#include "chromeos/settings/cros_settings_names.h"
#include "components/arc/arc_util.h"
#include "components/prefs/pref_service.h"

namespace {

const char kTestAppId[] = "ljoammodoonkhnehlncldjelhidljdpi";

}  // namespace

class ChromeOSInfoPrivateTest : public ExtensionApiTest {
 public:
  ChromeOSInfoPrivateTest() {}
  ~ChromeOSInfoPrivateTest() override {}

 protected:
  void EnableKioskSession() {
    base::CommandLine::ForCurrentProcess()->AppendSwitch(
        switches::kForceAppMode);
    base::CommandLine::ForCurrentProcess()->AppendSwitchASCII(switches::kAppId,
                                                              kTestAppId);
  }

  void SetDeviceType(const std::string& device_type) {
    const std::string lsb_release = std::string("DEVICETYPE=") + device_type;
    base::SysInfo::SetChromeOSVersionInfoForTest(lsb_release,
                                                 base::Time::Now());
  }
};

IN_PROC_BROWSER_TEST_F(ChromeOSInfoPrivateTest, TestGetAndSet) {
  // Set the initial timezone different from what JS function
  // timezoneSetTest() will attempt to set.
  profile()->GetPrefs()->SetString(prefs::kUserTimezone, "America/Los_Angeles");

  // Check that accessibility settings are set to default values.
  PrefService* prefs = profile()->GetPrefs();
  ASSERT_FALSE(prefs->GetBoolean(ash::prefs::kAccessibilityLargeCursorEnabled));
  ASSERT_FALSE(prefs->GetBoolean(ash::prefs::kAccessibilityStickyKeysEnabled));
  ASSERT_FALSE(
      prefs->GetBoolean(ash::prefs::kAccessibilitySpokenFeedbackEnabled));
  ASSERT_FALSE(
      prefs->GetBoolean(ash::prefs::kAccessibilityHighContrastEnabled));
  ASSERT_FALSE(
      prefs->GetBoolean(ash::prefs::kAccessibilityScreenMagnifierEnabled));
  ASSERT_FALSE(prefs->GetBoolean(ash::prefs::kAccessibilityAutoclickEnabled));

  ASSERT_FALSE(profile()->GetPrefs()->GetBoolean(
      prefs::kLanguageSendFunctionKeys));

  ASSERT_TRUE(RunComponentExtensionTest("chromeos_info_private/basic"))
      << message_;

  // Check that all accessibility settings have been flipped by the test.
  ASSERT_TRUE(prefs->GetBoolean(ash::prefs::kAccessibilityLargeCursorEnabled));
  ASSERT_TRUE(prefs->GetBoolean(ash::prefs::kAccessibilityStickyKeysEnabled));
  ASSERT_TRUE(
      prefs->GetBoolean(ash::prefs::kAccessibilitySpokenFeedbackEnabled));
  ASSERT_TRUE(prefs->GetBoolean(ash::prefs::kAccessibilityHighContrastEnabled));
  ASSERT_TRUE(
      prefs->GetBoolean(ash::prefs::kAccessibilityScreenMagnifierEnabled));
  ASSERT_TRUE(prefs->GetBoolean(ash::prefs::kAccessibilityAutoclickEnabled));

  ASSERT_TRUE(prefs->GetBoolean(prefs::kLanguageSendFunctionKeys));
}

// TODO(steel): Investigate merging the following tests.

IN_PROC_BROWSER_TEST_F(ChromeOSInfoPrivateTest, Kiosk) {
  EnableKioskSession();
  ASSERT_TRUE(
      RunPlatformAppTestWithArg("chromeos_info_private/extended", "kiosk"))
      << message_;
}

IN_PROC_BROWSER_TEST_F(ChromeOSInfoPrivateTest, ArcNotAvailable) {
  ASSERT_TRUE(RunPlatformAppTestWithArg("chromeos_info_private/extended",
                                        "arc not-available"))
      << message_;
}

IN_PROC_BROWSER_TEST_F(ChromeOSInfoPrivateTest, Chromebase) {
  SetDeviceType("CHROMEBASE");
  ASSERT_TRUE(
      RunPlatformAppTestWithArg("chromeos_info_private/extended", "chromebase"))
      << message_;
}

IN_PROC_BROWSER_TEST_F(ChromeOSInfoPrivateTest, Chromebit) {
  SetDeviceType("CHROMEBIT");
  ASSERT_TRUE(
      RunPlatformAppTestWithArg("chromeos_info_private/extended", "chromebit"))
      << message_;
}
IN_PROC_BROWSER_TEST_F(ChromeOSInfoPrivateTest, Chromebook) {
  SetDeviceType("CHROMEBOOK");
  ASSERT_TRUE(
      RunPlatformAppTestWithArg("chromeos_info_private/extended", "chromebook"))
      << message_;
}

IN_PROC_BROWSER_TEST_F(ChromeOSInfoPrivateTest, Chromebox) {
  SetDeviceType("CHROMEBOX");
  ASSERT_TRUE(
      RunPlatformAppTestWithArg("chromeos_info_private/extended", "chromebox"))
      << message_;
}

IN_PROC_BROWSER_TEST_F(ChromeOSInfoPrivateTest, UnknownDeviceType) {
  SetDeviceType("UNKNOWN");
  ASSERT_TRUE(RunPlatformAppTestWithArg("chromeos_info_private/extended",
                                        "unknown device type"))
      << message_;
}

class ChromeOSArcInfoPrivateTest : public ChromeOSInfoPrivateTest {
 public:
  ChromeOSArcInfoPrivateTest() = default;
  ~ChromeOSArcInfoPrivateTest() override = default;

  void SetUpCommandLine(base::CommandLine* command_line) override {
    ExtensionApiTest::SetUpCommandLine(command_line);
    // Make ARC enabled for ArcAvailable/ArcEnabled tests.
    arc::SetArcAvailableCommandLineForTesting(command_line);
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ChromeOSArcInfoPrivateTest);
};

IN_PROC_BROWSER_TEST_F(ChromeOSArcInfoPrivateTest, ArcEnabled) {
  ASSERT_TRUE(RunPlatformAppTestWithArg("chromeos_info_private/extended",
                                        "arc enabled"))
      << message_;
}

IN_PROC_BROWSER_TEST_F(ChromeOSArcInfoPrivateTest, ArcAvailable) {
  // Even if ARC is available, ARC may not be able to be enabled. (Please
  // see arc::IsArcAllowedForProfile() for details).
  // In such cases, we expect "available". However, current testing framework
  // does not seem to run with such cases, unfortunately. So, here directly
  // control the function.
  arc::DisallowArcForTesting();
  ASSERT_TRUE(RunPlatformAppTestWithArg("chromeos_info_private/extended",
                                        "arc available"))
      << message_;
}

class ChromeOSManagedDeviceInfoPrivateTest : public ChromeOSInfoPrivateTest {
 public:
  ChromeOSManagedDeviceInfoPrivateTest() = default;
  ~ChromeOSManagedDeviceInfoPrivateTest() override = default;

 protected:
  void SetUpInProcessBrowserTestFixture() override {
    // Set up fake install attributes.
    std::unique_ptr<chromeos::StubInstallAttributes> attributes =
        base::MakeUnique<chromeos::StubInstallAttributes>();
    attributes->SetCloudManaged("fake-domain", "fake-id");
    policy::BrowserPolicyConnectorChromeOS::SetInstallAttributesForTesting(
        attributes.release());
    ChromeOSInfoPrivateTest::SetUpInProcessBrowserTestFixture();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ChromeOSManagedDeviceInfoPrivateTest);
};

IN_PROC_BROWSER_TEST_F(ChromeOSManagedDeviceInfoPrivateTest, Managed) {
  ASSERT_TRUE(
      RunPlatformAppTestWithArg("chromeos_info_private/extended", "managed"))
      << message_;
}

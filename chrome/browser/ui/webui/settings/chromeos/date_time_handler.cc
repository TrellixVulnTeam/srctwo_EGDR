// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/settings/chromeos/date_time_handler.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/values.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/chromeos/policy/proto/chrome_device_policy.pb.h"
#include "chrome/browser/chromeos/set_time_dialog.h"
#include "chrome/browser/chromeos/system/timezone_resolver_manager.h"
#include "chrome/browser/chromeos/system/timezone_util.h"
#include "chrome/common/pref_names.h"
#include "chromeos/chromeos_switches.h"
#include "chromeos/dbus/dbus_thread_manager.h"
#include "chromeos/dbus/system_clock_client.h"
#include "chromeos/settings/timezone_settings.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"

namespace chromeos {
namespace settings {

namespace {

// Returns whether the system time zone automatic detection policy is disabled
// by a flag.
bool IsSystemTimezoneAutomaticDetectionPolicyFlagDisabled() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(
      switches::kDisableSystemTimezoneAutomaticDetectionPolicy);
}

// Returns whether the system's automatic time zone detection setting is
// managed, which may override the user's setting.
bool IsSystemTimezoneAutomaticDetectionManaged() {
  if (IsSystemTimezoneAutomaticDetectionPolicyFlagDisabled())
    return false;

  return g_browser_process->local_state()->IsManagedPreference(
      prefs::kSystemTimezoneAutomaticDetectionPolicy);
}

// Returns the system's automatic time zone detection policy value, which
// corresponds to the SystemTimezoneProto's AutomaticTimezoneDetectionType
// enum and determines whether the user's setting will be overridden.
int GetSystemTimezoneAutomaticDetectionPolicyValue() {
  DCHECK(IsSystemTimezoneAutomaticDetectionManaged());

  return g_browser_process->local_state()->GetInteger(
      prefs::kSystemTimezoneAutomaticDetectionPolicy);
}

// Returns whether the user can set the automatic detection setting, based on
// flags and policies.
bool IsTimezoneAutomaticDetectionUserEditable() {
  if (system::HasSystemTimezonePolicy())
    return false;

  if (IsSystemTimezoneAutomaticDetectionManaged()) {
    return GetSystemTimezoneAutomaticDetectionPolicyValue() ==
        enterprise_management::SystemTimezoneProto::USERS_DECIDE;
  }

  return true;
}

}  // namespace

DateTimeHandler::DateTimeHandler()
    : scoped_observer_(this), weak_ptr_factory_(this) {}

DateTimeHandler::~DateTimeHandler() = default;

DateTimeHandler* DateTimeHandler::Create(
    content::WebUIDataSource* html_source) {
  // Set the initial time zone to show.
  html_source->AddString("timeZoneName", system::GetCurrentTimezoneName());
  html_source->AddString(
      "timeZoneID",
      system::TimezoneSettings::GetInstance()->GetCurrentTimezoneID());

  if (!IsTimezoneAutomaticDetectionUserEditable()) {
    html_source->AddBoolean("timeZoneAutoDetectValueFromPolicy",
                            g_browser_process->platform_part()
                                ->GetTimezoneResolverManager()
                                ->ShouldApplyResolvedTimezone());
  }

  return new DateTimeHandler;
}

void DateTimeHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "dateTimePageReady", base::Bind(&DateTimeHandler::HandleDateTimePageReady,
                                      base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "getTimeZones",
      base::Bind(&DateTimeHandler::HandleGetTimeZones, base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "showSetDateTimeUI", base::Bind(&DateTimeHandler::HandleShowSetDateTimeUI,
                                      base::Unretained(this)));
}

void DateTimeHandler::OnJavascriptAllowed() {
  SystemClockClient* system_clock_client =
      DBusThreadManager::Get()->GetSystemClockClient();
  scoped_observer_.Add(system_clock_client);
  SystemClockCanSetTimeChanged(system_clock_client->CanSetTime());

  // The system time zone policy disables auto-detection entirely. (However,
  // the time zone policy does not override the user's time zone itself.)
  system_timezone_policy_subscription_ =
      CrosSettings::Get()->AddSettingsObserver(
          kSystemTimezonePolicy,
          base::Bind(&DateTimeHandler::NotifyTimezoneAutomaticDetectionPolicy,
                     weak_ptr_factory_.GetWeakPtr()));

  if (IsSystemTimezoneAutomaticDetectionPolicyFlagDisabled())
    return;

  // The auto-detection policy can force auto-detection on or off.
  local_state_pref_change_registrar_.Init(g_browser_process->local_state());
  local_state_pref_change_registrar_.Add(
      prefs::kSystemTimezoneAutomaticDetectionPolicy,
      base::Bind(&DateTimeHandler::NotifyTimezoneAutomaticDetectionPolicy,
                 base::Unretained(this)));
}

void DateTimeHandler::OnJavascriptDisallowed() {
  scoped_observer_.RemoveAll();
  system_timezone_policy_subscription_.reset();

  if (!IsSystemTimezoneAutomaticDetectionPolicyFlagDisabled())
    local_state_pref_change_registrar_.RemoveAll();
}

void DateTimeHandler::HandleDateTimePageReady(const base::ListValue* args) {
  AllowJavascript();

  // Send the time zone automatic detection policy in case it changed after the
  // handler was created.
  NotifyTimezoneAutomaticDetectionPolicy();
}

void DateTimeHandler::HandleGetTimeZones(const base::ListValue* args) {
  AllowJavascript();

  CHECK_EQ(1U, args->GetSize());
  const base::Value* callback_id;
  CHECK(args->Get(0, &callback_id));
  ResolveJavascriptCallback(*callback_id, *system::GetTimezoneList().release());
}

void DateTimeHandler::HandleShowSetDateTimeUI(const base::ListValue* args) {
  // Make sure the clock status hasn't changed since the button was clicked.
  if (!DBusThreadManager::Get()->GetSystemClockClient()->CanSetTime())
    return;
  SetTimeDialog::ShowDialogInParent(
      web_ui()->GetWebContents()->GetTopLevelNativeWindow());
}

void DateTimeHandler::NotifyTimezoneAutomaticDetectionPolicy() {
  bool managed = !IsTimezoneAutomaticDetectionUserEditable();
  bool force_enabled = managed &&
                       g_browser_process->platform_part()
                           ->GetTimezoneResolverManager()
                           ->ShouldApplyResolvedTimezone();

  FireWebUIListener("time-zone-auto-detect-policy", base::Value(managed),
                    base::Value(force_enabled));
}

void DateTimeHandler::SystemClockCanSetTimeChanged(bool can_set_time) {
  FireWebUIListener("can-set-date-time-changed", base::Value(can_set_time));
}

}  // namespace settings
}  // namespace chromeos

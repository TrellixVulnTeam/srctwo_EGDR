// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/signin/easy_unlock_screenlock_state_handler.h"

#include <stddef.h>

#include "base/bind.h"
#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "chrome/browser/signin/easy_unlock_metrics.h"
#include "chrome/grit/generated_resources.h"
#include "ui/base/l10n/l10n_util.h"

#if defined(OS_CHROMEOS)
#include "ui/chromeos/devicetype_utils.h"
#endif

using proximity_auth::ScreenlockState;

namespace {

proximity_auth::ScreenlockBridge::UserPodCustomIcon GetIconForState(
    ScreenlockState state) {
  switch (state) {
    case ScreenlockState::NO_BLUETOOTH:
    case ScreenlockState::NO_PHONE:
    case ScreenlockState::PHONE_NOT_AUTHENTICATED:
    case ScreenlockState::PHONE_LOCKED:
    case ScreenlockState::PHONE_NOT_LOCKABLE:
    case ScreenlockState::PHONE_UNSUPPORTED:
      return proximity_auth::ScreenlockBridge::USER_POD_CUSTOM_ICON_LOCKED;
    case ScreenlockState::RSSI_TOO_LOW:
    case ScreenlockState::PHONE_LOCKED_AND_RSSI_TOO_LOW:
      // TODO(isherman): This icon is currently identical to the regular locked
      // icon.  Once the reduced proximity range flag is removed, consider
      // deleting the redundant icon.
      return proximity_auth::ScreenlockBridge::
          USER_POD_CUSTOM_ICON_LOCKED_WITH_PROXIMITY_HINT;
    case ScreenlockState::BLUETOOTH_CONNECTING:
      return proximity_auth::ScreenlockBridge::USER_POD_CUSTOM_ICON_SPINNER;
    case ScreenlockState::AUTHENTICATED:
      return proximity_auth::ScreenlockBridge::USER_POD_CUSTOM_ICON_UNLOCKED;
    case ScreenlockState::INACTIVE:
      return proximity_auth::ScreenlockBridge::USER_POD_CUSTOM_ICON_NONE;
    case ScreenlockState::PASSWORD_REAUTH:
      return proximity_auth::ScreenlockBridge::USER_POD_CUSTOM_ICON_HARDLOCKED;
  }

  NOTREACHED();
  return proximity_auth::ScreenlockBridge::USER_POD_CUSTOM_ICON_NONE;
}

bool HardlockOnClick(ScreenlockState state) {
  return state != ScreenlockState::INACTIVE;
}

size_t GetTooltipResourceId(ScreenlockState state) {
  switch (state) {
    case ScreenlockState::INACTIVE:
    case ScreenlockState::BLUETOOTH_CONNECTING:
      return 0;
    case ScreenlockState::NO_BLUETOOTH:
      return IDS_EASY_UNLOCK_SCREENLOCK_TOOLTIP_NO_BLUETOOTH;
    case ScreenlockState::NO_PHONE:
      return IDS_EASY_UNLOCK_SCREENLOCK_TOOLTIP_NO_PHONE;
    case ScreenlockState::PHONE_NOT_AUTHENTICATED:
      return IDS_EASY_UNLOCK_SCREENLOCK_TOOLTIP_PHONE_NOT_AUTHENTICATED;
    case ScreenlockState::PHONE_LOCKED:
      return IDS_EASY_UNLOCK_SCREENLOCK_TOOLTIP_PHONE_LOCKED;
    case ScreenlockState::PHONE_NOT_LOCKABLE:
      return IDS_EASY_UNLOCK_SCREENLOCK_TOOLTIP_PHONE_UNLOCKABLE;
    case ScreenlockState::PHONE_UNSUPPORTED:
      return IDS_EASY_UNLOCK_SCREENLOCK_TOOLTIP_UNSUPPORTED_ANDROID_VERSION;
    case ScreenlockState::RSSI_TOO_LOW:
      return IDS_EASY_UNLOCK_SCREENLOCK_TOOLTIP_RSSI_TOO_LOW;
    case ScreenlockState::PHONE_LOCKED_AND_RSSI_TOO_LOW:
      return IDS_EASY_UNLOCK_SCREENLOCK_TOOLTIP_PHONE_LOCKED_AND_RSSI_TOO_LOW;
    case ScreenlockState::AUTHENTICATED:
      return IDS_EASY_UNLOCK_SCREENLOCK_TOOLTIP_HARDLOCK_INSTRUCTIONS;
    case ScreenlockState::PASSWORD_REAUTH:
      return IDS_EASY_UNLOCK_SCREENLOCK_TOOLTIP_PASSWORD_REAUTH;
  }

  NOTREACHED();
  return 0;
}

bool TooltipContainsDeviceType(ScreenlockState state) {
  return (state == ScreenlockState::AUTHENTICATED ||
          state == ScreenlockState::PHONE_NOT_LOCKABLE ||
          state == ScreenlockState::NO_BLUETOOTH ||
          state == ScreenlockState::PHONE_UNSUPPORTED ||
          state == ScreenlockState::RSSI_TOO_LOW ||
          state == ScreenlockState::PHONE_LOCKED_AND_RSSI_TOO_LOW);
}

// Returns true iff the |state| corresponds to a locked remote device.
bool IsLockedState(ScreenlockState state) {
  return (state == ScreenlockState::PHONE_LOCKED ||
          state == ScreenlockState::PHONE_LOCKED_AND_RSSI_TOO_LOW);
}

}  // namespace

EasyUnlockScreenlockStateHandler::EasyUnlockScreenlockStateHandler(
    const AccountId& account_id,
    HardlockState initial_hardlock_state,
    proximity_auth::ScreenlockBridge* screenlock_bridge)
    : state_(ScreenlockState::INACTIVE),
      account_id_(account_id),
      screenlock_bridge_(screenlock_bridge),
      hardlock_state_(initial_hardlock_state) {
  DCHECK(screenlock_bridge_);
  screenlock_bridge_->AddObserver(this);
}

EasyUnlockScreenlockStateHandler::~EasyUnlockScreenlockStateHandler() {
  screenlock_bridge_->RemoveObserver(this);
  // Make sure the screenlock state set by this gets cleared.
  ChangeState(ScreenlockState::INACTIVE);
}

bool EasyUnlockScreenlockStateHandler::IsActive() const {
  return state_ != ScreenlockState::INACTIVE;
}

bool EasyUnlockScreenlockStateHandler::InStateValidOnRemoteAuthFailure() const {
  // Note that NO_PHONE is not valid in this case because the phone may close
  // the connection if the auth challenge sent to it is invalid. This case
  // should be handled as authentication failure.
  return (state_ == ScreenlockState::NO_BLUETOOTH ||
          state_ == ScreenlockState::PHONE_LOCKED);
}

void EasyUnlockScreenlockStateHandler::ChangeState(ScreenlockState new_state) {
  if (state_ == new_state)
    return;

  state_ = new_state;

  // If lock screen is not active or it forces offline password, just cache the
  // current state. The screenlock state will get refreshed in |ScreenDidLock|.
  if (!screenlock_bridge_->IsLocked())
    return;

  // Do nothing when auth type is online.
  if (screenlock_bridge_->lock_handler()->GetAuthType(account_id_) ==
      proximity_auth::mojom::AuthType::ONLINE_SIGN_IN) {
    return;
  }

  if (IsLockedState(state_))
    did_see_locked_phone_ = true;

  // No hardlock UI for trial run.
  if (!is_trial_run_ && hardlock_state_ != NO_HARDLOCK) {
    ShowHardlockUI();
    return;
  }

  UpdateScreenlockAuthType();

  proximity_auth::ScreenlockBridge::UserPodCustomIcon icon =
      GetIconForState(state_);

  if (icon == proximity_auth::ScreenlockBridge::USER_POD_CUSTOM_ICON_NONE) {
    screenlock_bridge_->lock_handler()->HideUserPodCustomIcon(account_id_);
    return;
  }

  proximity_auth::ScreenlockBridge::UserPodCustomIconOptions icon_options;
  icon_options.SetIcon(icon);

  // Don't hardlock on trial run.
  if (is_trial_run_)
    icon_options.SetTrialRun();
  else if (HardlockOnClick(state_))
    icon_options.SetHardlockOnClick();

  UpdateTooltipOptions(&icon_options);

  // For states without tooltips, we still need to set an accessibility label.
  if (state_ == ScreenlockState::BLUETOOTH_CONNECTING) {
    icon_options.SetAriaLabel(
        l10n_util::GetStringUTF16(IDS_SMART_LOCK_SPINNER_ACCESSIBILITY_LABEL));
  }

  screenlock_bridge_->lock_handler()->ShowUserPodCustomIcon(account_id_,
                                                            icon_options);
}

void EasyUnlockScreenlockStateHandler::SetHardlockState(
    HardlockState new_state) {
  if (hardlock_state_ == new_state)
    return;

  if (new_state == LOGIN_FAILED && hardlock_state_ != NO_HARDLOCK)
    return;

  hardlock_state_ = new_state;

  // If hardlock_state_ was set to NO_HARDLOCK, this means the screen is about
  // to get unlocked. No need to update it in this case.
  if (hardlock_state_ != NO_HARDLOCK) {
    hardlock_ui_shown_ = false;

    RefreshScreenlockState();
  }
}

void EasyUnlockScreenlockStateHandler::MaybeShowHardlockUI() {
  if (hardlock_state_ != NO_HARDLOCK)
    ShowHardlockUI();
}

void EasyUnlockScreenlockStateHandler::SetTrialRun() {
  if (is_trial_run_)
    return;
  is_trial_run_ = true;
  RefreshScreenlockState();
  RecordEasyUnlockTrialRunEvent(EASY_UNLOCK_TRIAL_RUN_EVENT_LAUNCHED);
}

void EasyUnlockScreenlockStateHandler::RecordClickOnLockIcon() {
  if (!is_trial_run_)
    return;
  RecordEasyUnlockTrialRunEvent(EASY_UNLOCK_TRIAL_RUN_EVENT_CLICKED_LOCK_ICON);
}

void EasyUnlockScreenlockStateHandler::OnScreenDidLock(
    proximity_auth::ScreenlockBridge::LockHandler::ScreenType screen_type) {
  did_see_locked_phone_ = IsLockedState(state_);
  RefreshScreenlockState();
}

void EasyUnlockScreenlockStateHandler::OnScreenDidUnlock(
    proximity_auth::ScreenlockBridge::LockHandler::ScreenType screen_type) {
  if (hardlock_state_ == LOGIN_FAILED)
    hardlock_state_ = NO_HARDLOCK;
  hardlock_ui_shown_ = false;
  is_trial_run_ = false;

  // Upon a successful unlock event, record whether the user's phone was locked
  // at any point while the lock screen was up.
  if (state_ == ScreenlockState::AUTHENTICATED)
    RecordEasyUnlockDidUserManuallyUnlockPhone(did_see_locked_phone_);
  did_see_locked_phone_ = false;
}

void EasyUnlockScreenlockStateHandler::OnFocusedUserChanged(
    const AccountId& account_id) {}

void EasyUnlockScreenlockStateHandler::RefreshScreenlockState() {
  ScreenlockState last_state = state_;
  // This should force updating screenlock state.
  state_ = ScreenlockState::INACTIVE;
  ChangeState(last_state);
}

void EasyUnlockScreenlockStateHandler::ShowHardlockUI() {
  DCHECK(hardlock_state_ != NO_HARDLOCK);

  if (!screenlock_bridge_->IsLocked())
    return;

  // Do not override online signin.
  const proximity_auth::mojom::AuthType existing_auth_type =
      screenlock_bridge_->lock_handler()->GetAuthType(account_id_);
  if (existing_auth_type == proximity_auth::mojom::AuthType::ONLINE_SIGN_IN)
    return;

  if (existing_auth_type != proximity_auth::mojom::AuthType::OFFLINE_PASSWORD) {
    screenlock_bridge_->lock_handler()->SetAuthType(
        account_id_, proximity_auth::mojom::AuthType::OFFLINE_PASSWORD,
        base::string16());
  }

  if (hardlock_state_ == NO_PAIRING) {
    screenlock_bridge_->lock_handler()->HideUserPodCustomIcon(account_id_);
    hardlock_ui_shown_ = false;
    return;
  }

  if (hardlock_ui_shown_)
    return;

  proximity_auth::ScreenlockBridge::UserPodCustomIconOptions icon_options;
  if (hardlock_state_ == LOGIN_FAILED) {
    icon_options.SetIcon(
        proximity_auth::ScreenlockBridge::USER_POD_CUSTOM_ICON_LOCKED);
  } else if (hardlock_state_ == PAIRING_CHANGED ||
             hardlock_state_ == PAIRING_ADDED) {
    icon_options.SetIcon(proximity_auth::ScreenlockBridge::
                             USER_POD_CUSTOM_ICON_LOCKED_TO_BE_ACTIVATED);
  } else {
    icon_options.SetIcon(
        proximity_auth::ScreenlockBridge::USER_POD_CUSTOM_ICON_HARDLOCKED);
  }

  base::string16 device_name = GetDeviceName();
  base::string16 tooltip;
  if (hardlock_state_ == USER_HARDLOCK) {
    tooltip = l10n_util::GetStringFUTF16(
        IDS_EASY_UNLOCK_SCREENLOCK_TOOLTIP_HARDLOCK_USER, device_name);
  } else if (hardlock_state_ == PAIRING_CHANGED) {
    tooltip = l10n_util::GetStringFUTF16(
        IDS_EASY_UNLOCK_SCREENLOCK_TOOLTIP_HARDLOCK_PAIRING_CHANGED,
        device_name);
  } else if (hardlock_state_ == PAIRING_ADDED) {
    tooltip = l10n_util::GetStringFUTF16(
        IDS_EASY_UNLOCK_SCREENLOCK_TOOLTIP_HARDLOCK_PAIRING_ADDED, device_name);
  } else if (hardlock_state_ == LOGIN_FAILED) {
    tooltip = l10n_util::GetStringUTF16(
        IDS_EASY_UNLOCK_SCREENLOCK_TOOLTIP_LOGIN_FAILURE);
  } else if (hardlock_state_ == PASSWORD_REQUIRED_FOR_LOGIN) {
    tooltip = l10n_util::GetStringFUTF16(
        IDS_EASY_UNLOCK_SCREENLOCK_TOOLTIP_PASSWORD_REQUIRED_FOR_LOGIN,
        device_name);
  } else {
    LOG(ERROR) << "Unknown hardlock state " << hardlock_state_;
  }
  icon_options.SetTooltip(tooltip, true /* autoshow */);

  screenlock_bridge_->lock_handler()->ShowUserPodCustomIcon(account_id_,
                                                            icon_options);
  hardlock_ui_shown_ = true;
}

void EasyUnlockScreenlockStateHandler::UpdateTooltipOptions(
    proximity_auth::ScreenlockBridge::UserPodCustomIconOptions* icon_options) {
  size_t resource_id = 0;
  base::string16 device_name;
  if (is_trial_run_ && state_ == ScreenlockState::AUTHENTICATED) {
    resource_id = IDS_EASY_UNLOCK_SCREENLOCK_TOOLTIP_INITIAL_AUTHENTICATED;
  } else {
    resource_id = GetTooltipResourceId(state_);
    if (TooltipContainsDeviceType(state_))
      device_name = GetDeviceName();
  }

  if (!resource_id)
    return;

  base::string16 tooltip;
  if (device_name.empty()) {
    tooltip = l10n_util::GetStringUTF16(resource_id);
  } else {
    tooltip = l10n_util::GetStringFUTF16(resource_id, device_name);
  }

  if (tooltip.empty())
    return;

  bool autoshow_tooltip =
      is_trial_run_ || state_ != ScreenlockState::AUTHENTICATED;
  icon_options->SetTooltip(tooltip, autoshow_tooltip);
}

base::string16 EasyUnlockScreenlockStateHandler::GetDeviceName() {
#if defined(OS_CHROMEOS)
  return ui::GetChromeOSDeviceName();
#else
  // TODO(tbarzic): Figure out the name for non Chrome OS case.
  return base::ASCIIToUTF16("Chrome");
#endif
}

void EasyUnlockScreenlockStateHandler::UpdateScreenlockAuthType() {
  if (!is_trial_run_ && hardlock_state_ != NO_HARDLOCK)
    return;

  // Do not override online signin.
  const proximity_auth::mojom::AuthType existing_auth_type =
      screenlock_bridge_->lock_handler()->GetAuthType(account_id_);
  DCHECK_NE(proximity_auth::mojom::AuthType::ONLINE_SIGN_IN,
            existing_auth_type);

  if (state_ == ScreenlockState::AUTHENTICATED) {
    if (existing_auth_type != proximity_auth::mojom::AuthType::USER_CLICK) {
      screenlock_bridge_->lock_handler()->SetAuthType(
          account_id_, proximity_auth::mojom::AuthType::USER_CLICK,
          l10n_util::GetStringUTF16(
              IDS_EASY_UNLOCK_SCREENLOCK_USER_POD_AUTH_VALUE));
    }
  } else if (existing_auth_type !=
             proximity_auth::mojom::AuthType::OFFLINE_PASSWORD) {
    screenlock_bridge_->lock_handler()->SetAuthType(
        account_id_, proximity_auth::mojom::AuthType::OFFLINE_PASSWORD,
        base::string16());
  }
}

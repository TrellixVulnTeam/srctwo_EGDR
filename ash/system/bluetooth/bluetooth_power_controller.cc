// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/system/bluetooth/bluetooth_power_controller.h"

#include "ash/public/cpp/ash_pref_names.h"
#include "ash/session/session_controller.h"
#include "ash/shell.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "device/bluetooth/bluetooth_adapter_factory.h"

namespace ash {

BluetoothPowerController::BluetoothPowerController() : weak_ptr_factory_(this) {
  device::BluetoothAdapterFactory::GetAdapter(
      base::Bind(&BluetoothPowerController::InitializeOnAdapterReady,
                 weak_ptr_factory_.GetWeakPtr()));
  Shell::Get()->AddShellObserver(this);
  Shell::Get()->session_controller()->AddObserver(this);
}

BluetoothPowerController::~BluetoothPowerController() {
  if (bluetooth_adapter_)
    bluetooth_adapter_->RemoveObserver(this);
  Shell::Get()->RemoveShellObserver(this);
  Shell::Get()->session_controller()->RemoveObserver(this);
}

void BluetoothPowerController::ToggleBluetoothEnabled() {
  if (active_user_pref_service_) {
    active_user_pref_service_->SetBoolean(
        prefs::kUserBluetoothAdapterEnabled,
        !active_user_pref_service_->GetBoolean(
            prefs::kUserBluetoothAdapterEnabled));
  } else if (local_state_pref_service_) {
    local_state_pref_service_->SetBoolean(
        prefs::kSystemBluetoothAdapterEnabled,
        !local_state_pref_service_->GetBoolean(
            prefs::kSystemBluetoothAdapterEnabled));
  } else {
    DLOG(ERROR)
        << "active user and local state pref service cannot both be null";
  }
}

// static
void BluetoothPowerController::RegisterLocalStatePrefs(
    PrefRegistrySimple* registry) {
  registry->RegisterBooleanPref(prefs::kSystemBluetoothAdapterEnabled, false);
}

// static
void BluetoothPowerController::RegisterProfilePrefs(
    PrefRegistrySimple* registry) {
  registry->RegisterBooleanPref(prefs::kUserBluetoothAdapterEnabled, false,
                                PrefRegistry::PUBLIC);
}

void BluetoothPowerController::StartWatchingActiveUserPrefsChanges() {
  DCHECK(active_user_pref_service_);
  DCHECK(Shell::Get()->session_controller()->IsUserPrimary());

  active_user_pref_change_registrar_ = base::MakeUnique<PrefChangeRegistrar>();
  active_user_pref_change_registrar_->Init(active_user_pref_service_);
  active_user_pref_change_registrar_->Add(
      prefs::kUserBluetoothAdapterEnabled,
      base::Bind(
          &BluetoothPowerController::OnBluetoothPowerActiveUserPrefChanged,
          base::Unretained(this)));
}

void BluetoothPowerController::StartWatchingLocalStatePrefsChanges() {
  DCHECK(local_state_pref_service_);

  local_state_pref_change_registrar_ = base::MakeUnique<PrefChangeRegistrar>();
  local_state_pref_change_registrar_->Init(local_state_pref_service_);
  local_state_pref_change_registrar_->Add(
      prefs::kSystemBluetoothAdapterEnabled,
      base::Bind(
          &BluetoothPowerController::OnBluetoothPowerLocalStatePrefChanged,
          base::Unretained(this)));
}

void BluetoothPowerController::StopWatchingActiveUserPrefsChanges() {
  active_user_pref_change_registrar_.reset();
}

void BluetoothPowerController::OnBluetoothPowerActiveUserPrefChanged() {
  DCHECK(active_user_pref_service_);
  SetBluetoothPower(active_user_pref_service_->GetBoolean(
      prefs::kUserBluetoothAdapterEnabled));
}

void BluetoothPowerController::OnBluetoothPowerLocalStatePrefChanged() {
  DCHECK(local_state_pref_service_);
  SetBluetoothPower(local_state_pref_service_->GetBoolean(
      prefs::kSystemBluetoothAdapterEnabled));
}

void BluetoothPowerController::SetPrimaryUserBluetoothPowerSetting(
    bool enabled) {
  // This method should only be called when the primary user is the active user.
  CHECK(Shell::Get()->session_controller()->IsUserPrimary());

  active_user_pref_service_->SetBoolean(prefs::kUserBluetoothAdapterEnabled,
                                        enabled);
}

void BluetoothPowerController::InitializeOnAdapterReady(
    scoped_refptr<device::BluetoothAdapter> adapter) {
  bluetooth_adapter_ = std::move(adapter);
  bluetooth_adapter_->AddObserver(this);
  if (bluetooth_adapter_->IsPresent()) {
    TriggerRunPendingBluetoothTasks();
  }
}

void BluetoothPowerController::OnActiveUserPrefServiceChanged(
    PrefService* pref_service) {
  active_user_pref_service_ = pref_service;

  // Only listen to primary user's pref changes since non-primary users
  // are not able to change bluetooth pref.
  if (!Shell::Get()->session_controller()->IsUserPrimary()) {
    StopWatchingActiveUserPrefsChanges();
    return;
  }
  StartWatchingActiveUserPrefsChanges();

  // Apply the bluetooth pref only for regular users (i.e. users representing
  // a human individual). We don't want to apply bluetooth pref for other users
  // e.g. kiosk, guest etc. For non-human users, bluetooth power should be left
  // to the current power state.
  if (!is_primary_user_bluetooth_applied_) {
    ApplyBluetoothPrimaryUserPref();
    is_primary_user_bluetooth_applied_ = true;
  }
}

void BluetoothPowerController::OnLocalStatePrefServiceInitialized(
    PrefService* pref_service) {
  // AppLaunchTest.TestQuickLaunch fails under target=linux due to
  // pref_service being nullptr.
  if (!pref_service)
    return;

  local_state_pref_service_ = pref_service;

  StartWatchingLocalStatePrefsChanges();

  if (!Shell::Get()->session_controller()->IsActiveUserSessionStarted()) {
    // Apply the local state pref only if no user has logged in (still in login
    // screen).
    ApplyBluetoothLocalStatePref();
  }
}

void BluetoothPowerController::AdapterPresentChanged(
    device::BluetoothAdapter* adapter,
    bool present) {
  if (present)
    TriggerRunPendingBluetoothTasks();
}

void BluetoothPowerController::ApplyBluetoothPrimaryUserPref() {
  base::Optional<user_manager::UserType> user_type =
      Shell::Get()->session_controller()->GetUserType();
  if (!user_type || !ShouldApplyUserBluetoothSetting(*user_type)) {
    // Do not apply bluetooth setting if user is not of the allowed types.
    return;
  }

  DCHECK(Shell::Get()->session_controller()->IsUserPrimary());

  PrefService* prefs = active_user_pref_service_;

  if (!prefs->FindPreference(prefs::kUserBluetoothAdapterEnabled)
           ->IsDefaultValue()) {
    SetBluetoothPower(prefs->GetBoolean(prefs::kUserBluetoothAdapterEnabled));
    return;
  }

  // If the user has not had the bluetooth pref yet, set the user pref
  // according to whatever the current bluetooth power is, except for
  // new users (first login on the device) always set the new pref to true.
  if (Shell::Get()->session_controller()->IsUserFirstLogin()) {
    prefs->SetBoolean(prefs::kUserBluetoothAdapterEnabled, true);
  } else {
    SavePrefValue(prefs, prefs::kUserBluetoothAdapterEnabled);
  }
}

void BluetoothPowerController::ApplyBluetoothLocalStatePref() {
  PrefService* prefs = local_state_pref_service_;

  if (prefs->FindPreference(prefs::kSystemBluetoothAdapterEnabled)
          ->IsDefaultValue()) {
    // If the device has not had the local state bluetooth pref, set the pref
    // according to whatever the current bluetooth power is.
    SavePrefValue(prefs, prefs::kSystemBluetoothAdapterEnabled);
  } else {
    SetBluetoothPower(prefs->GetBoolean(prefs::kSystemBluetoothAdapterEnabled));
  }
}

void BluetoothPowerController::SetBluetoothPower(bool enabled) {
  RunBluetoothTaskWhenAdapterReady(
      base::BindOnce(&BluetoothPowerController::SetBluetoothPowerOnAdapterReady,
                     weak_ptr_factory_.GetWeakPtr(), enabled));
}

void BluetoothPowerController::SetBluetoothPowerOnAdapterReady(bool enabled) {
  // Always run the next pending task after SetPowered completes regardless
  // the error.
  bluetooth_adapter_->SetPowered(
      enabled,
      base::Bind(&BluetoothPowerController::RunNextPendingBluetoothTask,
                 weak_ptr_factory_.GetWeakPtr()),
      base::Bind(&BluetoothPowerController::RunNextPendingBluetoothTask,
                 weak_ptr_factory_.GetWeakPtr()));
}

void BluetoothPowerController::RunBluetoothTaskWhenAdapterReady(
    BluetoothTask task) {
  pending_bluetooth_tasks_.push(std::move(task));
  TriggerRunPendingBluetoothTasks();
}

void BluetoothPowerController::TriggerRunPendingBluetoothTasks() {
  if (pending_tasks_busy_)
    return;
  pending_tasks_busy_ = true;
  RunNextPendingBluetoothTask();
}

void BluetoothPowerController::RunNextPendingBluetoothTask() {
  if (!bluetooth_adapter_ || !bluetooth_adapter_->IsPresent() ||
      pending_bluetooth_tasks_.empty()) {
    // Stop running pending tasks if either adapter becomes not present or
    // all the pending tasks have been run.
    pending_tasks_busy_ = false;
    return;
  }
  BluetoothTask task = std::move(pending_bluetooth_tasks_.front());
  pending_bluetooth_tasks_.pop();
  std::move(task).Run();
}

void BluetoothPowerController::SavePrefValue(PrefService* prefs,
                                             const char* pref_name) {
  RunBluetoothTaskWhenAdapterReady(
      base::BindOnce(&BluetoothPowerController::SavePrefValueOnAdapterReady,
                     weak_ptr_factory_.GetWeakPtr(), prefs, pref_name));
}

void BluetoothPowerController::SavePrefValueOnAdapterReady(
    PrefService* prefs,
    const char* pref_name) {
  prefs->SetBoolean(pref_name, bluetooth_adapter_->IsPowered());
  RunNextPendingBluetoothTask();
}

bool BluetoothPowerController::ShouldApplyUserBluetoothSetting(
    user_manager::UserType user_type) const {
  return user_type == user_manager::USER_TYPE_REGULAR ||
         user_type == user_manager::USER_TYPE_CHILD ||
         user_type == user_manager::USER_TYPE_SUPERVISED ||
         user_type == user_manager::USER_TYPE_ACTIVE_DIRECTORY;
}

}  // namespace ash

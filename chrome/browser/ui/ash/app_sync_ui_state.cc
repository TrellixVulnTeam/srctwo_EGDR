// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/ash/app_sync_ui_state.h"

#include "build/build_config.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/pending_extension_manager.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/sync/profile_sync_service_factory.h"
#include "chrome/browser/ui/ash/app_sync_ui_state_factory.h"
#include "chrome/browser/ui/ash/app_sync_ui_state_observer.h"
#include "components/browser_sync/profile_sync_service.h"
#include "components/prefs/pref_service.h"
#include "components/user_manager/user_manager.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_system.h"

namespace {

// Max loading animation time in milliseconds.
const int kMaxSyncingTimeMs = 60 * 1000;

}  // namespace

// static
AppSyncUIState* AppSyncUIState::Get(Profile* profile) {
  return AppSyncUIStateFactory::GetForProfile(profile);
}

// static
bool AppSyncUIState::ShouldObserveAppSyncForProfile(Profile* profile) {
  if (user_manager::UserManager::Get()->IsLoggedInAsGuest())
    return false;

  if (!profile || profile->IsOffTheRecord())
    return false;

  if (!ProfileSyncServiceFactory::HasProfileSyncService(profile))
    return false;

  return profile->IsNewProfile();
}

AppSyncUIState::AppSyncUIState(Profile* profile)
    : profile_(profile),
      sync_service_(NULL),
      status_(STATUS_NORMAL),
      extension_registry_(NULL) {
  StartObserving();
}

AppSyncUIState::~AppSyncUIState() {
  StopObserving();
}

void AppSyncUIState::AddObserver(AppSyncUIStateObserver* observer) {
  observers_.AddObserver(observer);
}

void AppSyncUIState::RemoveObserver(AppSyncUIStateObserver* observer) {
  observers_.RemoveObserver(observer);
}

void AppSyncUIState::StartObserving() {
  DCHECK(ShouldObserveAppSyncForProfile(profile_));
  DCHECK(!sync_service_);
  DCHECK(!extension_registry_);

  extension_registry_ = extensions::ExtensionRegistry::Get(profile_);
  extension_registry_->AddObserver(this);

  sync_service_ = ProfileSyncServiceFactory::GetForProfile(profile_);
  CHECK(sync_service_);
  sync_service_->AddObserver(this);
}

void AppSyncUIState::StopObserving() {
  if (!sync_service_)
    return;

  sync_service_->RemoveObserver(this);
  sync_service_ = NULL;

  if (extension_registry_)
    extension_registry_->RemoveObserver(this);
  extension_registry_ = NULL;

  profile_ = NULL;
}

void AppSyncUIState::SetStatus(Status status) {
  if (status_ == status)
    return;

  status_ = status;
  switch (status_) {
    case STATUS_SYNCING:
      max_syncing_status_timer_.Start(
          FROM_HERE,
          base::TimeDelta::FromMilliseconds(kMaxSyncingTimeMs),
          this, &AppSyncUIState::OnMaxSyncingTimer);
      break;
    case STATUS_NORMAL:
    case STATUS_TIMED_OUT:
      max_syncing_status_timer_.Stop();
      StopObserving();
      break;
  }

  for (AppSyncUIStateObserver& observer : observers_)
    observer.OnAppSyncUIStatusChanged();
}

void AppSyncUIState::CheckAppSync() {
  if (!sync_service_ || !sync_service_->IsFirstSetupComplete())
    return;

  const bool synced = sync_service_->IsSyncActive();
  const bool has_pending_extension =
      extensions::ExtensionSystem::Get(profile_)->extension_service()->
          pending_extension_manager()->HasPendingExtensionFromSync();

  if (synced && !has_pending_extension)
    SetStatus(STATUS_NORMAL);
  else
    SetStatus(STATUS_SYNCING);
}

void AppSyncUIState::OnMaxSyncingTimer() {
  SetStatus(STATUS_TIMED_OUT);
}

void AppSyncUIState::OnStateChanged(syncer::SyncService* sync) {
  DCHECK(sync_service_);
  CheckAppSync();
}

void AppSyncUIState::OnExtensionLoaded(content::BrowserContext* browser_context,
                                       const extensions::Extension* extension) {
  CheckAppSync();
}

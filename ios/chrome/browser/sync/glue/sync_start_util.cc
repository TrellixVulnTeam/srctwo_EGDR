// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/chrome/browser/sync/glue/sync_start_util.h"

#include "base/bind.h"
#include "base/files/file_path.h"
#include "base/location.h"
#include "components/browser_sync/profile_sync_service.h"
#include "components/sync/driver/sync_service.h"
#include "ios/chrome/browser/application_context.h"
#include "ios/chrome/browser/browser_state/chrome_browser_state.h"
#include "ios/chrome/browser/browser_state/chrome_browser_state_manager.h"
#include "ios/chrome/browser/sync/ios_chrome_profile_sync_service_factory.h"
#include "ios/web/public/web_thread.h"

namespace ios {
namespace {

void StartSyncOnUIThread(const base::FilePath& browser_state_path,
                         syncer::ModelType type) {
  ios::ChromeBrowserStateManager* browser_state_manager =
      GetApplicationContext()->GetChromeBrowserStateManager();
  if (!browser_state_manager) {
    // Can happen in tests.
    DVLOG(2) << "No ChromeBrowserStateManager, can't start sync.";
    return;
  }

  ios::ChromeBrowserState* browser_state =
      browser_state_manager->GetBrowserState(browser_state_path);
  if (!browser_state) {
    DVLOG(2) << "ChromeBrowserState not found, can't start sync.";
    return;
  }

  syncer::SyncService* sync_service =
      IOSChromeProfileSyncServiceFactory::GetForBrowserState(browser_state);
  if (!sync_service) {
    DVLOG(2) << "No SyncService for browser state, can't start sync.";
    return;
  }
  sync_service->OnDataTypeRequestsSyncStartup(type);
}

void StartSyncProxy(const base::FilePath& browser_state_path,
                    syncer::ModelType type) {
  web::WebThread::PostTask(
      web::WebThread::UI, FROM_HERE,
      base::Bind(&StartSyncOnUIThread, browser_state_path, type));
}

}  // namespace

namespace sync_start_util {

syncer::SyncableService::StartSyncFlare GetFlareForSyncableService(
    const base::FilePath& browser_state_path) {
  return base::Bind(&StartSyncProxy, browser_state_path);
}

}  // namespace sync_start_util
}  // namespace ios

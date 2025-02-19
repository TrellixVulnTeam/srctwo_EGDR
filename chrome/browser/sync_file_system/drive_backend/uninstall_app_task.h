// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SYNC_FILE_SYSTEM_DRIVE_BACKEND_UNINSTALL_APP_TASK_H_
#define CHROME_BROWSER_SYNC_FILE_SYSTEM_DRIVE_BACKEND_UNINSTALL_APP_TASK_H_

#include <stdint.h>

#include <memory>
#include <string>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/sync_file_system/drive_backend/sync_task.h"
#include "chrome/browser/sync_file_system/remote_file_sync_service.h"
#include "chrome/browser/sync_file_system/sync_callbacks.h"
#include "google_apis/drive/drive_api_error_codes.h"

namespace drive {
class DriveServiceInterface;
}

namespace sync_file_system {
namespace drive_backend {

class MetadataDatabase;
class SyncEngineContext;

class UninstallAppTask : public ExclusiveTask {
 public:
  typedef RemoteFileSyncService::UninstallFlag UninstallFlag;
  UninstallAppTask(SyncEngineContext* sync_context,
                   const std::string& app_id,
                   UninstallFlag uninstall_flag);
  ~UninstallAppTask() override;

  void RunExclusive(const SyncStatusCallback& callback) override;

 private:
  void DidDeleteAppRoot(const SyncStatusCallback& callback,
                        int64_t change_id,
                        google_apis::DriveApiErrorCode error);

  bool IsContextReady();
  MetadataDatabase* metadata_database();
  drive::DriveServiceInterface* drive_service();

  SyncEngineContext* sync_context_;  // Not owned.

  std::string app_id_;
  UninstallFlag uninstall_flag_;
  int64_t app_root_tracker_id_;

  base::WeakPtrFactory<UninstallAppTask> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(UninstallAppTask);
};

}  // namespace drive_backend
}  // namespace sync_file_system

#endif  // CHROME_BROWSER_SYNC_FILE_SYSTEM_DRIVE_BACKEND_UNINSTALL_APP_TASK_H_

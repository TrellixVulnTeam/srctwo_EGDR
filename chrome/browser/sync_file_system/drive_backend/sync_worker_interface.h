// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SYNC_FILE_SYSTEM_DRIVE_BACKEND_SYNC_WORKER_INTERFACE_H_
#define CHROME_BROWSER_SYNC_FILE_SYSTEM_DRIVE_BACKEND_SYNC_WORKER_INTERFACE_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "chrome/browser/sync_file_system/remote_file_sync_service.h"
#include "chrome/browser/sync_file_system/sync_action.h"
#include "chrome/browser/sync_file_system/sync_callbacks.h"
#include "chrome/browser/sync_file_system/sync_direction.h"
#include "net/base/network_change_notifier.h"

class GURL;

namespace base {
class FilePath;
class ListValue;
}

namespace storage {
class FileSystemURL;
}

namespace sync_file_system {

class FileChange;
class SyncFileMetadata;

namespace drive_backend {

class RemoteChangeProcessorOnWorker;
class SyncEngineContext;

class SyncWorkerInterface {
 public:
  class Observer {
   public:
    virtual void OnPendingFileListUpdated(int item_count) = 0;
    virtual void OnFileStatusChanged(const storage::FileSystemURL& url,
                                     SyncFileType file_type,
                                     SyncFileStatus file_status,
                                     SyncAction sync_action,
                                     SyncDirection direction) = 0;
    virtual void UpdateServiceState(RemoteServiceState state,
                                    const std::string& description) = 0;

   protected:
    virtual ~Observer() {}
  };

  SyncWorkerInterface() {}
  virtual ~SyncWorkerInterface() {}

  // Initializes SyncWorkerInterface after constructions of some member classes.
  virtual void Initialize(
      std::unique_ptr<SyncEngineContext> sync_engine_context) = 0;

  // See RemoteFileSyncService for the details.
  virtual void RegisterOrigin(const GURL& origin,
                              const SyncStatusCallback& callback) = 0;
  virtual void EnableOrigin(const GURL& origin,
                            const SyncStatusCallback& callback) = 0;
  virtual void DisableOrigin(const GURL& origin,
                             const SyncStatusCallback& callback) = 0;
  virtual void UninstallOrigin(
      const GURL& origin,
      RemoteFileSyncService::UninstallFlag flag,
      const SyncStatusCallback& callback) = 0;
  virtual void ProcessRemoteChange(const SyncFileCallback& callback) = 0;
  virtual void SetRemoteChangeProcessor(
      RemoteChangeProcessorOnWorker* remote_change_processor_on_worker) = 0;
  virtual RemoteServiceState GetCurrentState() const = 0;
  virtual void GetOriginStatusMap(
      const RemoteFileSyncService::StatusMapCallback& callback) = 0;
  virtual std::unique_ptr<base::ListValue> DumpFiles(const GURL& origin) = 0;
  virtual std::unique_ptr<base::ListValue> DumpDatabase() = 0;
  virtual void SetSyncEnabled(bool enabled) = 0;
  virtual void PromoteDemotedChanges(const base::Closure& callback) = 0;

  // See LocalChangeProcessor for the details.
  virtual void ApplyLocalChange(const FileChange& local_change,
                                const base::FilePath& local_path,
                                const SyncFileMetadata& local_metadata,
                                const storage::FileSystemURL& url,
                                const SyncStatusCallback& callback) = 0;

  virtual void ActivateService(RemoteServiceState service_state,
                               const std::string& description) = 0;
  virtual void DeactivateService(const std::string& description) = 0;

  virtual void DetachFromSequence() = 0;

  virtual void AddObserver(Observer* observer) = 0;

 private:
  friend class SyncEngineTest;

  DISALLOW_COPY_AND_ASSIGN(SyncWorkerInterface);
};

}  // namespace drive_backend
}  // namespace sync_file_system

#endif  // CHROME_BROWSER_SYNC_FILE_SYSTEM_DRIVE_BACKEND_SYNC_WORKER_INTERFACE_H_

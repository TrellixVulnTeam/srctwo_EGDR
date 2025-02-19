// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_MEDIA_GALLERIES_CHROMEOS_MTP_DEVICE_DELEGATE_IMPL_CHROMEOS_H_
#define CHROME_BROWSER_MEDIA_GALLERIES_CHROMEOS_MTP_DEVICE_DELEGATE_IMPL_CHROMEOS_H_

#include <stdint.h>

#include <map>
#include <memory>
#include <set>
#include <string>

#include "base/callback.h"
#include "base/containers/circular_deque.h"
#include "base/files/file_path.h"
#include "base/location.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/media_galleries/chromeos/mtp_device_task_helper.h"
#include "chrome/browser/media_galleries/fileapi/mtp_device_async_delegate.h"
#include "content/public/browser/browser_thread.h"
#include "storage/browser/fileapi/async_file_util.h"

struct SnapshotRequestInfo;

// MTPDeviceDelegateImplLinux communicates with the media transfer protocol
// (MTP) device to complete file system operations. These operations are
// performed asynchronously. Instantiate this class per MTP device storage.
// MTPDeviceDelegateImplLinux lives on the IO thread.
// MTPDeviceDelegateImplLinux does a call-and-reply to the UI thread
// to dispatch the requests to MediaTransferProtocolManager.
class MTPDeviceDelegateImplLinux : public MTPDeviceAsyncDelegate {
 private:
  friend void CreateMTPDeviceAsyncDelegate(
      const std::string&,
      const bool read_only,
      const CreateMTPDeviceAsyncDelegateCallback&);

  enum InitializationState {
    UNINITIALIZED = 0,
    PENDING_INIT,
    INITIALIZED
  };

  // Used to represent pending task details.
  struct PendingTaskInfo {
    PendingTaskInfo(const base::FilePath& path,
                    content::BrowserThread::ID thread_id,
                    const tracked_objects::Location& location,
                    const base::Closure& task);
    PendingTaskInfo(const PendingTaskInfo& other);
    ~PendingTaskInfo();

    base::FilePath path;
    base::FilePath cached_path;
    const content::BrowserThread::ID thread_id;
    const tracked_objects::Location location;
    const base::Closure task;
  };

  class MTPFileNode;

  // Maps file ids to file nodes.
  typedef std::map<uint32_t, MTPFileNode*> FileIdToMTPFileNodeMap;

  // Maps file paths to file info.
  typedef std::map<base::FilePath, MTPDeviceTaskHelper::MTPEntry> FileInfoCache;

  typedef base::Closure DeleteObjectSuccessCallback;

  // Should only be called by CreateMTPDeviceAsyncDelegate() factory call.
  // Defer the device initializations until the first file operation request.
  // Do all the initializations in EnsureInitAndRunTask() function.
  MTPDeviceDelegateImplLinux(const std::string& device_location,
                             const bool read_only);

  // Destructed via CancelPendingTasksAndDeleteDelegate().
  ~MTPDeviceDelegateImplLinux() override;

  // MTPDeviceAsyncDelegate:
  void GetFileInfo(const base::FilePath& file_path,
                   const GetFileInfoSuccessCallback& success_callback,
                   const ErrorCallback& error_callback) override;
  void CreateDirectory(const base::FilePath& directory_path,
                       const bool exclusive,
                       const bool recursive,
                       const CreateDirectorySuccessCallback& success_callback,
                       const ErrorCallback& error_callback) override;
  void ReadDirectory(const base::FilePath& root,
                     const ReadDirectorySuccessCallback& success_callback,
                     const ErrorCallback& error_callback) override;
  void CreateSnapshotFile(
      const base::FilePath& device_file_path,
      const base::FilePath& local_path,
      const CreateSnapshotFileSuccessCallback& success_callback,
      const ErrorCallback& error_callback) override;
  bool IsStreaming() override;
  void ReadBytes(const base::FilePath& device_file_path,
                 const scoped_refptr<net::IOBuffer>& buf,
                 int64_t offset,
                 int buf_len,
                 const ReadBytesSuccessCallback& success_callback,
                 const ErrorCallback& error_callback) override;
  bool IsReadOnly() const override;
  void CopyFileLocal(
      const base::FilePath& source_file_path,
      const base::FilePath& device_file_path,
      const CreateTemporaryFileCallback& create_temporary_file_callback,
      const CopyFileProgressCallback& progress_callback,
      const CopyFileLocalSuccessCallback& success_callback,
      const ErrorCallback& error_callback) override;
  void MoveFileLocal(
      const base::FilePath& source_file_path,
      const base::FilePath& device_file_path,
      const CreateTemporaryFileCallback& create_temporary_file_callback,
      const MoveFileLocalSuccessCallback& success_callback,
      const ErrorCallback& error_callback) override;
  void CopyFileFromLocal(
      const base::FilePath& source_file_path,
      const base::FilePath& device_file_path,
      const CopyFileFromLocalSuccessCallback& success_callback,
      const ErrorCallback& error_callback) override;
  void DeleteFile(const base::FilePath& file_path,
                  const DeleteFileSuccessCallback& success_callback,
                  const ErrorCallback& error_callback) override;
  void DeleteDirectory(const base::FilePath& file_path,
                       const DeleteDirectorySuccessCallback& success_callback,
                       const ErrorCallback& error_callback) override;
  void AddWatcher(const GURL& origin,
                  const base::FilePath& file_path,
                  const bool recursive,
                  const storage::WatcherManager::StatusCallback& callback,
                  const storage::WatcherManager::NotificationCallback&
                      notification_callback) override;
  void RemoveWatcher(
      const GURL& origin,
      const base::FilePath& file_path,
      const bool recursive,
      const storage::WatcherManager::StatusCallback& callback) override;
  void CancelPendingTasksAndDeleteDelegate() override;

  // The internal methods correspond to the similarly named methods above.
  // The |root_node_| cache should be filled at this point.
  virtual void GetFileInfoInternal(
      const base::FilePath& file_path,
      const GetFileInfoSuccessCallback& success_callback,
      const ErrorCallback& error_callback);
  virtual void CreateDirectoryInternal(
      const std::vector<base::FilePath>& components,
      const bool exclusive,
      const CreateDirectorySuccessCallback& success_callback,
      const ErrorCallback& error_callback);
  virtual void ReadDirectoryInternal(
      const base::FilePath& root,
      const ReadDirectorySuccessCallback& success_callback,
      const ErrorCallback& error_callback);
  virtual void CreateSnapshotFileInternal(
      const base::FilePath& device_file_path,
      const base::FilePath& local_path,
      const CreateSnapshotFileSuccessCallback& success_callback,
      const ErrorCallback& error_callback);
  virtual void ReadBytesInternal(
      const base::FilePath& device_file_path,
      net::IOBuffer* buf,
      int64_t offset,
      int buf_len,
      const ReadBytesSuccessCallback& success_callback,
      const ErrorCallback& error_callback);
  virtual void MoveFileLocalInternal(
      const base::FilePath& source_file_path,
      const base::FilePath& device_file_path,
      const CreateTemporaryFileCallback& create_temporary_file_callback,
      const MoveFileLocalSuccessCallback& success_callback,
      const ErrorCallback& error_callback,
      const base::File::Info& source_file_info);
  virtual void OnDidOpenFDToCopyFileFromLocal(
      const base::FilePath& device_file_path,
      const CopyFileFromLocalSuccessCallback& success_callback,
      const ErrorCallback& error_callback,
      const std::pair<int, base::File::Error>& open_fd_result);
  virtual void DeleteFileInternal(
      const base::FilePath& file_path,
      const DeleteFileSuccessCallback& success_callback,
      const ErrorCallback& error_callback,
      const base::File::Info& file_info);
  virtual void DeleteDirectoryInternal(
      const base::FilePath& file_path,
      const DeleteDirectorySuccessCallback& success_callback,
      const ErrorCallback& error_callback,
      const base::File::Info& file_info);

  // Creates a single directory to |directory_path|. The caller must ensure that
  // parent directory |directory_path.DirName()| already exists.
  virtual void CreateSingleDirectory(
      const base::FilePath& directory_path,
      const bool exclusive,
      const CreateDirectorySuccessCallback& success_callback,
      const ErrorCallback& error_callback);

  // Called when ReadDirectoryInternal() completes for filling cache as part of
  // creating directories.
  virtual void OnDidReadDirectoryToCreateDirectory(
      const std::vector<base::FilePath>& components,
      const bool exclusive,
      const CreateDirectorySuccessCallback& success_callback,
      const ErrorCallback& error_callback,
      storage::AsyncFileUtil::EntryList entries,
      const bool has_more);

  // Called when ReadDirectory succeeds.
  virtual void OnDidReadDirectoryToDeleteDirectory(
      const base::FilePath& directory_path,
      const uint32_t directory_id,
      const DeleteDirectorySuccessCallback& success_callback,
      const ErrorCallback& error_callback,
      const MTPDeviceTaskHelper::MTPEntries& entries,
      const bool has_more);

  // Calls DeleteObjectOnUIThread on UI thread.
  virtual void RunDeleteObjectOnUIThread(
      const base::FilePath& object_path,
      const uint32_t object_id,
      const DeleteObjectSuccessCallback& success_callback,
      const ErrorCallback& error_callback);

  // Notifies |chage_type| of |file_path| to watchers.
  void NotifyFileChange(const base::FilePath& file_path,
                        const storage::WatcherManager::ChangeType change_type);

  // Ensures the device is initialized for communication.
  // If the device is already initialized, call RunTask().
  //
  // If the device is uninitialized, store the |task_info| in a pending task
  // queue and runs the pending tasks in the queue once the device is
  // successfully initialized.
  void EnsureInitAndRunTask(const PendingTaskInfo& task_info);

  // Runs a task. If |task_info.path| is empty, or if the path is cached, runs
  // the task immediately.
  // Otherwise, fills the cache first before running the task.
  // |task_info.task| runs on the UI thread.
  void RunTask(const PendingTaskInfo& task_info);

  // Writes data from the device to the snapshot file path based on the
  // parameters in |current_snapshot_request_info_| by doing a call-and-reply to
  // the UI thread.
  //
  // |snapshot_file_info| specifies the metadata details of the snapshot file.
  void WriteDataIntoSnapshotFile(const base::File::Info& snapshot_file_info);

  // Marks the current request as complete and call ProcessNextPendingRequest().
  void PendingRequestDone();

  // Processes the next pending request.
  void ProcessNextPendingRequest();

  // Handles the device initialization event. |succeeded| indicates whether
  // device initialization succeeded.
  //
  // If the device is successfully initialized, runs the next pending task.
  void OnInitCompleted(bool succeeded);

  // Called when GetFileInfo() succeeds. |file_info| specifies the
  // requested file details. |success_callback| is invoked to notify the caller
  // about the requested file details.
  void OnDidGetFileInfo(const GetFileInfoSuccessCallback& success_callback,
                        const base::File::Info& file_info);

  // Called when GetFileInfo() of |directory_path| succeeded at checking the
  // path already exists.
  void OnPathAlreadyExistsForCreateSingleDirectory(
      const bool exclusive,
      const CreateDirectorySuccessCallback& success_callback,
      const ErrorCallback& error_callback,
      const base::File::Info& file_info);

  // Called when GetFileInfo() of |directory_path| failed to check the path
  // already exists.
  void OnPathDoesNotExistForCreateSingleDirectory(
      const base::FilePath& directory_path,
      const CreateDirectorySuccessCallback& success_callback,
      const ErrorCallback& error_callback,
      const base::File::Error error);

  // Called when GetFileInfo() succeeds. GetFileInfo() is invoked to
  // get the |dir_id| directory metadata details. |file_info| specifies the
  // |dir_id| directory details.
  //
  // If |dir_id| is a directory, post a task on the UI thread to read the
  // |dir_id| directory file entries.
  //
  // If |dir_id| is not a directory, |error_callback| is invoked to notify the
  // caller about the file error and process the next pending request.
  void OnDidGetFileInfoToReadDirectory(
      uint32_t dir_id,
      const ReadDirectorySuccessCallback& success_callback,
      const ErrorCallback& error_callback,
      const base::File::Info& file_info);

  // Called when GetFileInfo() succeeds. GetFileInfo() is invoked to
  // create the snapshot file of |snapshot_request_info.device_file_path|.
  // |file_info| specifies the device file metadata details.
  //
  // Posts a task on the UI thread to copy the data contents of the device file
  // to the snapshot file.
  void OnDidGetFileInfoToCreateSnapshotFile(
      std::unique_ptr<SnapshotRequestInfo> snapshot_request_info,
      const base::File::Info& file_info);

  // Called when GetFileInfo() for destination path succeeded for a
  // CopyFileFromLocal operation.
  void OnDidGetDestFileInfoToCopyFileFromLocal(
      const ErrorCallback& error_callback,
      const base::File::Info& file_info);

  // Called when GetFileInfo() for destination path failed to copy file from
  // local.
  void OnGetDestFileInfoErrorToCopyFileFromLocal(
      const base::FilePath& source_file_path,
      const base::FilePath& device_file_path,
      const CopyFileFromLocalSuccessCallback& success_callback,
      const ErrorCallback& error_callback,
      const base::File::Error error);

  // Called when CreateSignleDirectory() succeeds.
  void OnDidCreateSingleDirectory(
      const base::FilePath& directory_path,
      const CreateDirectorySuccessCallback& success_callback);

  // Called when parent directory |created_directory| is created as part of
  // CreateDirectory.
  void OnDidCreateParentDirectoryToCreateDirectory(
      const base::FilePath& created_directory,
      const std::vector<base::FilePath>& components,
      const bool exclusive,
      const CreateDirectorySuccessCallback& success_callback,
      const ErrorCallback& error_callback);

  // Called when it failed to create a parent directory. For creating parent
  // directories, all errors should be reported as FILE_ERROR_FAILED. This
  // method wraps error callbacks of creating parent directories.
  void OnCreateParentDirectoryErrorToCreateDirectory(
      const ErrorCallback& callback,
      const base::File::Error error);

  // Called when ReadDirectory() succeeds.
  //
  // |dir_id| is the directory read.
  // |success_callback| is invoked to notify the caller about the directory
  // file entries.
  // |file_list| contains the directory file entries with their file ids.
  // |has_more| is true if there are more file entries to read.
  void OnDidReadDirectory(uint32_t dir_id,
                          const ReadDirectorySuccessCallback& success_callback,
                          const MTPDeviceTaskHelper::MTPEntries& mtp_entries,
                          bool has_more);

  // Called when WriteDataIntoSnapshotFile() succeeds.
  //
  // |snapshot_file_info| specifies the snapshot file metadata details.
  //
  // |current_snapshot_request_info_.success_callback| is invoked to notify the
  // caller about |snapshot_file_info|.
  void OnDidWriteDataIntoSnapshotFile(
      const base::File::Info& snapshot_file_info,
      const base::FilePath& snapshot_file_path);

  // Called when WriteDataIntoSnapshotFile() fails.
  //
  // |error| specifies the file error code.
  //
  // |current_snapshot_request_info_.error_callback| is invoked to notify the
  // caller about |error|.
  void OnWriteDataIntoSnapshotFileError(base::File::Error error);

  // Called when ReadBytes() succeeds.
  //
  // |success_callback| is invoked to notify the caller about the read bytes.
  // |bytes_read| is the number of bytes read.
  void OnDidReadBytes(const ReadBytesSuccessCallback& success_callback,
                      const base::File::Info& file_info, int bytes_read);

  // Called when FillFileCache() succeeds.
  void OnDidFillFileCache(const base::FilePath& path,
                          storage::AsyncFileUtil::EntryList /* entries */,
                          bool has_more);

  // Called when FillFileCache() fails.
  void OnFillFileCacheFailed(base::File::Error error);

  // Called when CreateTemporaryFile() completes for CopyFileLocal.
  void OnDidCreateTemporaryFileToCopyFileLocal(
      const base::FilePath& source_file_path,
      const base::FilePath& device_file_path,
      const CopyFileProgressCallback& progress_callback,
      const CopyFileLocalSuccessCallback& success_callback,
      const ErrorCallback& error_callback,
      const base::FilePath& temporary_file_path);

  // Called when CreateSnapshotFile() succeeds for CopyFileLocal.
  void OnDidCreateSnapshotFileOfCopyFileLocal(
      const base::FilePath& device_file_path,
      const CopyFileProgressCallback& progress_callback,
      const CopyFileLocalSuccessCallback& success_callback,
      const ErrorCallback& error_callback,
      const base::File::Info& file_info,
      const base::FilePath& temporary_file_path);

  // Called when CopyFileFromLocal() succeeds for CopyFileLocal.
  void OnDidCopyFileFromLocalOfCopyFileLocal(
      const CopyFileFromLocalSuccessCallback success_callback,
      const base::FilePath& temporary_file_path);

  // Called when MoveFileLocal() succeeds with rename operation.
  void OnDidMoveFileLocalWithRename(
      const MoveFileLocalSuccessCallback& success_callback,
      const base::FilePath& source_file_path,
      const uint32_t file_id);

  // Called when CopyFileFromLocal() succeeds.
  void OnDidCopyFileFromLocal(
      const CopyFileFromLocalSuccessCallback& success_callback,
      const base::FilePath& file_path,
      const int source_file_descriptor);

  // Called when CopyFileLocal() fails.
  void HandleCopyFileLocalError(const ErrorCallback& error_callback,
                                const base::FilePath& temporary_file_path,
                                const base::File::Error error);

  // Called when CopyFileFromLocal() fails.
  void HandleCopyFileFromLocalError(const ErrorCallback& error_callback,
                                    const int source_file_descriptor,
                                    base::File::Error error);

  // Called when DeleteObject() succeeds.
  void OnDidDeleteObject(const base::FilePath& object_path,
                         const uint32_t object_id,
                         const DeleteObjectSuccessCallback success_callback);

  // Called when DeleteFileOrDirectory() fails.
  void HandleDeleteFileOrDirectoryError(const ErrorCallback& error_callback,
                                        base::File::Error error);

  // Handles the device file |error| while operating on |file_id|.
  // |error_callback| is invoked to notify the caller about the file error.
  void HandleDeviceFileError(const ErrorCallback& error_callback,
                             uint32_t file_id,
                             base::File::Error error);

  // Given a full path, returns a non-empty sub-path that needs to be read into
  // the cache if such a uncached path exists.
  // |cached_path| is the portion of |path| that has had cache lookup attempts.
  base::FilePath NextUncachedPathComponent(
      const base::FilePath& path,
      const base::FilePath& cached_path) const;

  // Fills the file cache using the results from NextUncachedPathComponent().
  void FillFileCache(const base::FilePath& uncached_path);

  // Given a full path, if it exists in the cache, writes the file's id to |id|
  // and return true.
  bool CachedPathToId(const base::FilePath& path, uint32_t* id) const;

  // Evict the cache of |id|.
  void EvictCachedPathToId(const uint32_t id);

  // MTP device initialization state.
  InitializationState init_state_;

  // Used to make sure only one task is in progress at any time.
  // Otherwise the browser will try to send too many requests at once and
  // overload the device.
  bool task_in_progress_;

  // Registered file system device path. This path does not
  // correspond to a real device path (e.g. "/usb:2,2:81282").
  const base::FilePath device_path_;

  // MTP device storage name (e.g. "usb:2,2:81282").
  std::string storage_name_;

  // Mode for opening storage.
  const bool read_only_;

  // Maps for holding notification callbacks.
  typedef std::map<GURL, storage::WatcherManager::NotificationCallback>
      OriginNotificationCallbackMap;
  typedef std::map<base::FilePath, OriginNotificationCallbackMap> Subscribers;
  Subscribers subscribers_;

  // A list of pending tasks that needs to be run when the device is
  // initialized or when the current task in progress is complete.
  base::circular_deque<PendingTaskInfo> pending_tasks_;

  // Used to track the current snapshot file request. A snapshot file is created
  // incrementally. CreateSnapshotFile request reads the device file and writes
  // to the snapshot file in chunks. In order to retain the order of the
  // snapshot file requests, make sure there is only one active snapshot file
  // request at any time.
  std::unique_ptr<SnapshotRequestInfo> current_snapshot_request_info_;

  // A mapping for quick lookups into the |root_node_| tree structure. Since
  // |root_node_| contains pointers to this map, it must be declared after this
  // so destruction happens in the right order.
  FileIdToMTPFileNodeMap file_id_to_node_map_;

  // The root node of a tree-structure that caches the directory structure of
  // the MTP device.
  std::unique_ptr<MTPFileNode> root_node_;

  // A list of child nodes encountered while a ReadDirectory operation, which
  // can return results over multiple callbacks, is in progress.
  std::set<std::string> child_nodes_seen_;

  // A cache to store file metadata for file entries read during a ReadDirectory
  // operation. Used to service incoming GetFileInfo calls for the duration of
  // the ReadDirectory operation.
  FileInfoCache file_info_cache_;

  // For callbacks that may run after destruction.
  base::WeakPtrFactory<MTPDeviceDelegateImplLinux> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(MTPDeviceDelegateImplLinux);
};

#endif  // CHROME_BROWSER_MEDIA_GALLERIES_CHROMEOS_MTP_DEVICE_DELEGATE_IMPL_CHROMEOS_H_

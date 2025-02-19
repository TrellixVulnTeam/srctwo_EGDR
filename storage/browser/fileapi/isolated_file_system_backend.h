// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STORAGE_BROWSER_FILEAPI_ISOLATED_FILE_SYSTEM_BACKEND_H_
#define STORAGE_BROWSER_FILEAPI_ISOLATED_FILE_SYSTEM_BACKEND_H_

#include <stdint.h>

#include <memory>

#include "storage/browser/fileapi/file_system_backend.h"
#include "storage/browser/fileapi/task_runner_bound_observer_list.h"

namespace storage {

class AsyncFileUtilAdapter;

class IsolatedFileSystemBackend : public FileSystemBackend {
 public:
  IsolatedFileSystemBackend(bool use_for_type_native_local,
                            bool use_for_type_platform_app);
  ~IsolatedFileSystemBackend() override;

  // FileSystemBackend implementation.
  bool CanHandleType(FileSystemType type) const override;
  void Initialize(FileSystemContext* context) override;
  void ResolveURL(const FileSystemURL& url,
                  OpenFileSystemMode mode,
                  OpenFileSystemCallback callback) override;
  AsyncFileUtil* GetAsyncFileUtil(FileSystemType type) override;
  WatcherManager* GetWatcherManager(FileSystemType type) override;
  CopyOrMoveFileValidatorFactory* GetCopyOrMoveFileValidatorFactory(
      FileSystemType type,
      base::File::Error* error_code) override;
  FileSystemOperation* CreateFileSystemOperation(
      const FileSystemURL& url,
      FileSystemContext* context,
      base::File::Error* error_code) const override;
  bool SupportsStreaming(const FileSystemURL& url) const override;
  bool HasInplaceCopyImplementation(
      storage::FileSystemType type) const override;
  std::unique_ptr<storage::FileStreamReader> CreateFileStreamReader(
      const FileSystemURL& url,
      int64_t offset,
      int64_t max_bytes_to_read,
      const base::Time& expected_modification_time,
      FileSystemContext* context) const override;
  std::unique_ptr<FileStreamWriter> CreateFileStreamWriter(
      const FileSystemURL& url,
      int64_t offset,
      FileSystemContext* context) const override;
  FileSystemQuotaUtil* GetQuotaUtil() override;
  const UpdateObserverList* GetUpdateObservers(
      FileSystemType type) const override;
  const ChangeObserverList* GetChangeObservers(
      FileSystemType type) const override;
  const AccessObserverList* GetAccessObservers(
      FileSystemType type) const override;

 private:
  // Whether this object should handle native local filesystem types. Some
  // platforms (e.g. Chrome OS) may provide a different FileSystemBackend to
  // handle those types.
  const bool use_for_type_native_local_;

  // As above but for platform webapps.
  const bool use_for_type_platform_app_;

  std::unique_ptr<AsyncFileUtilAdapter> isolated_file_util_;
  std::unique_ptr<AsyncFileUtilAdapter> dragged_file_util_;
  std::unique_ptr<AsyncFileUtilAdapter> transient_file_util_;
};

}  // namespace storage

#endif  // STORAGE_BROWSER_FILEAPI_ISOLATED_FILE_SYSTEM_BACKEND_H_

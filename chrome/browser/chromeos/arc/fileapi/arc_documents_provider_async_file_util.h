// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_ARC_FILEAPI_ARC_DOCUMENTS_PROVIDER_ASYNC_FILE_UTIL_H_
#define CHROME_BROWSER_CHROMEOS_ARC_FILEAPI_ARC_DOCUMENTS_PROVIDER_ASYNC_FILE_UTIL_H_

#include <memory>

#include "base/callback.h"
#include "base/macros.h"
#include "storage/browser/fileapi/async_file_util.h"

namespace arc {

// The implementation of storage::AsyncFileUtil for media view.
//
// All of the methods must be called on the IO thread.
class ArcDocumentsProviderAsyncFileUtil : public storage::AsyncFileUtil {
 public:
  ArcDocumentsProviderAsyncFileUtil();
  ~ArcDocumentsProviderAsyncFileUtil() override;

  // storage::AsyncFileUtil overrides.
  void CreateOrOpen(
      std::unique_ptr<storage::FileSystemOperationContext> context,
      const storage::FileSystemURL& url,
      int file_flags,
      const CreateOrOpenCallback& callback) override;
  void EnsureFileExists(
      std::unique_ptr<storage::FileSystemOperationContext> context,
      const storage::FileSystemURL& url,
      const EnsureFileExistsCallback& callback) override;
  void CreateDirectory(
      std::unique_ptr<storage::FileSystemOperationContext> context,
      const storage::FileSystemURL& url,
      bool exclusive,
      bool recursive,
      const StatusCallback& callback) override;
  void GetFileInfo(std::unique_ptr<storage::FileSystemOperationContext> context,
                   const storage::FileSystemURL& url,
                   int fields,
                   const GetFileInfoCallback& callback) override;
  void ReadDirectory(
      std::unique_ptr<storage::FileSystemOperationContext> context,
      const storage::FileSystemURL& url,
      const ReadDirectoryCallback& callback) override;
  void Touch(std::unique_ptr<storage::FileSystemOperationContext> context,
             const storage::FileSystemURL& url,
             const base::Time& last_access_time,
             const base::Time& last_modified_time,
             const StatusCallback& callback) override;
  void Truncate(std::unique_ptr<storage::FileSystemOperationContext> context,
                const storage::FileSystemURL& url,
                int64_t length,
                const StatusCallback& callback) override;
  void CopyFileLocal(
      std::unique_ptr<storage::FileSystemOperationContext> context,
      const storage::FileSystemURL& src_url,
      const storage::FileSystemURL& dest_url,
      CopyOrMoveOption option,
      const CopyFileProgressCallback& progress_callback,
      const StatusCallback& callback) override;
  void MoveFileLocal(
      std::unique_ptr<storage::FileSystemOperationContext> context,
      const storage::FileSystemURL& src_url,
      const storage::FileSystemURL& dest_url,
      CopyOrMoveOption option,
      const StatusCallback& callback) override;
  void CopyInForeignFile(
      std::unique_ptr<storage::FileSystemOperationContext> context,
      const base::FilePath& src_file_path,
      const storage::FileSystemURL& dest_url,
      const StatusCallback& callback) override;
  void DeleteFile(std::unique_ptr<storage::FileSystemOperationContext> context,
                  const storage::FileSystemURL& url,
                  const StatusCallback& callback) override;
  void DeleteDirectory(
      std::unique_ptr<storage::FileSystemOperationContext> context,
      const storage::FileSystemURL& url,
      const StatusCallback& callback) override;
  void DeleteRecursively(
      std::unique_ptr<storage::FileSystemOperationContext> context,
      const storage::FileSystemURL& url,
      const StatusCallback& callback) override;
  void CreateSnapshotFile(
      std::unique_ptr<storage::FileSystemOperationContext> context,
      const storage::FileSystemURL& url,
      const CreateSnapshotFileCallback& callback) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(ArcDocumentsProviderAsyncFileUtil);
};

}  // namespace arc

#endif  // CHROME_BROWSER_CHROMEOS_ARC_FILEAPI_ARC_DOCUMENTS_PROVIDER_ASYNC_FILE_UTIL_H_

// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STORAGE_BROWSER_FILEAPI_REMOVE_OPERATION_DELEGATE_H_
#define STORAGE_BROWSER_FILEAPI_REMOVE_OPERATION_DELEGATE_H_

#include <stack>

#include "base/macros.h"
#include "storage/browser/fileapi/recursive_operation_delegate.h"

namespace storage {

class RemoveOperationDelegate : public RecursiveOperationDelegate {
 public:
  RemoveOperationDelegate(FileSystemContext* file_system_context,
                          const FileSystemURL& url,
                          const StatusCallback& callback);
  ~RemoveOperationDelegate() override;

  // RecursiveOperationDelegate overrides:
  void Run() override;
  void RunRecursively() override;
  void ProcessFile(const FileSystemURL& url,
                   const StatusCallback& callback) override;
  void ProcessDirectory(const FileSystemURL& url,
                        const StatusCallback& callback) override;
  void PostProcessDirectory(const FileSystemURL& url,
                            const StatusCallback& callback) override;

 private:
  void DidTryRemoveFile(base::File::Error error);
  void DidTryRemoveDirectory(base::File::Error remove_file_error,
                             base::File::Error remove_directory_error);
  void DidRemoveFile(const StatusCallback& callback,
                     base::File::Error error);

  FileSystemURL url_;
  StatusCallback callback_;
  base::WeakPtrFactory<RemoveOperationDelegate> weak_factory_;
  DISALLOW_COPY_AND_ASSIGN(RemoveOperationDelegate);
};

}  // namespace storage

#endif  // STORAGE_BROWSER_FILEAPI_REMOVE_OPERATION_DELEGATE_H_

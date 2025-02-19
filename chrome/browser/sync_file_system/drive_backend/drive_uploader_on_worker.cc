// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/sync_file_system/drive_backend/drive_uploader_on_worker.h"

#include <string>

#include "base/bind.h"
#include "base/location.h"
#include "base/single_thread_task_runner.h"
#include "chrome/browser/sync_file_system/drive_backend/callback_helper.h"
#include "chrome/browser/sync_file_system/drive_backend/drive_uploader_wrapper.h"
#include "google_apis/drive/drive_api_parser.h"

namespace sync_file_system {
namespace drive_backend {

DriveUploaderOnWorker::DriveUploaderOnWorker(
      const base::WeakPtr<DriveUploaderWrapper>& wrapper,
      base::SingleThreadTaskRunner* ui_task_runner,
      base::SequencedTaskRunner* worker_task_runner)
    : wrapper_(wrapper),
      ui_task_runner_(ui_task_runner),
      worker_task_runner_(worker_task_runner) {
  sequece_checker_.DetachFromSequence();
}

DriveUploaderOnWorker::~DriveUploaderOnWorker() {}

void DriveUploaderOnWorker::StartBatchProcessing() {
}
void DriveUploaderOnWorker::StopBatchProcessing() {
}

google_apis::CancelCallback DriveUploaderOnWorker::UploadNewFile(
    const std::string& parent_resource_id,
    const base::FilePath& local_file_path,
    const std::string& title,
    const std::string& content_type,
    const drive::UploadNewFileOptions& options,
    const drive::UploadCompletionCallback& callback,
    const google_apis::ProgressCallback& progress_callback) {
  DCHECK(sequece_checker_.CalledOnValidSequence());

  ui_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&DriveUploaderWrapper::UploadNewFile, wrapper_,
                     parent_resource_id, local_file_path, title, content_type,
                     options,
                     RelayCallbackToTaskRunner(worker_task_runner_.get(),
                                               FROM_HERE, callback)));

  return google_apis::CancelCallback();
}

google_apis::CancelCallback DriveUploaderOnWorker::UploadExistingFile(
    const std::string& resource_id,
    const base::FilePath& local_file_path,
    const std::string& content_type,
    const drive::UploadExistingFileOptions& options,
    const drive::UploadCompletionCallback& callback,
    const google_apis::ProgressCallback& progress_callback) {
  DCHECK(sequece_checker_.CalledOnValidSequence());

  ui_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&DriveUploaderWrapper::UploadExistingFile, wrapper_,
                     resource_id, local_file_path, content_type, options,
                     RelayCallbackToTaskRunner(worker_task_runner_.get(),
                                               FROM_HERE, callback)));

  return google_apis::CancelCallback();
}

google_apis::CancelCallback DriveUploaderOnWorker::ResumeUploadFile(
      const GURL& upload_location,
      const base::FilePath& local_file_path,
      const std::string& content_type,
      const drive::UploadCompletionCallback& callback,
      const google_apis::ProgressCallback& progress_callback) {
  NOTREACHED();
  return google_apis::CancelCallback();
}

}  // namespace drive_backend
}  // namespace sync_file_system

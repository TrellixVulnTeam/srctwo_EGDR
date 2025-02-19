// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/upload_list/upload_list.h"

#include <algorithm>
#include <iterator>
#include <utility>

#include "base/bind.h"
#include "base/task_scheduler/post_task.h"

UploadList::UploadInfo::UploadInfo(const std::string& upload_id,
                                   const base::Time& upload_time,
                                   const std::string& local_id,
                                   const base::Time& capture_time,
                                   State state)
    : upload_id(upload_id),
      upload_time(upload_time),
      local_id(local_id),
      capture_time(capture_time),
      state(state) {}

UploadList::UploadInfo::UploadInfo(const std::string& local_id,
                                   const base::Time& capture_time,
                                   State state,
                                   const base::string16& file_size)
    : local_id(local_id),
      capture_time(capture_time),
      state(state),
      file_size(file_size) {}

UploadList::UploadInfo::UploadInfo(const std::string& upload_id,
                                   const base::Time& upload_time)
    : upload_id(upload_id), upload_time(upload_time), state(State::Uploaded) {}

UploadList::UploadInfo::UploadInfo(const UploadInfo& upload_info)
    : upload_id(upload_info.upload_id),
      upload_time(upload_info.upload_time),
      local_id(upload_info.local_id),
      capture_time(upload_info.capture_time),
      state(upload_info.state),
      file_size(upload_info.file_size) {}

UploadList::UploadInfo::~UploadInfo() = default;

UploadList::UploadList() = default;

UploadList::~UploadList() = default;

void UploadList::Load(base::OnceClosure callback) {
  DCHECK(sequence_checker_.CalledOnValidSequence());
  callback_ = std::move(callback);
  base::PostTaskWithTraitsAndReplyWithResult(
      FROM_HERE, LoadingTaskTraits(),
      base::Bind(&UploadList::LoadUploadList, this),
      base::Bind(&UploadList::OnLoadComplete, this));
}

void UploadList::CancelCallback() {
  callback_.Reset();
}

void UploadList::RequestSingleUploadAsync(const std::string& local_id) {
  DCHECK(sequence_checker_.CalledOnValidSequence());
  base::PostTaskWithTraits(
      FROM_HERE, LoadingTaskTraits(),
      base::Bind(&UploadList::RequestSingleUpload, this, local_id));
}

void UploadList::GetUploads(size_t max_count,
                            std::vector<UploadInfo>* uploads) {
  DCHECK(sequence_checker_.CalledOnValidSequence());
  std::copy(uploads_.begin(),
            uploads_.begin() + std::min(uploads_.size(), max_count),
            std::back_inserter(*uploads));
}

void UploadList::RequestSingleUpload(const std::string& local_id) {
  // Manual uploads for not-yet uploaded crash reports are only available for
  // Crashpad systems and for Android.
  NOTREACHED();
}

void UploadList::OnLoadComplete(const std::vector<UploadInfo>& uploads) {
  uploads_ = uploads;
  if (!callback_.is_null())
    std::move(callback_).Run();
}

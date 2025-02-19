// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/file_reader.h"

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/files/file_util.h"
#include "base/threading/thread_task_runner_handle.h"
#include "extensions/browser/extension_file_task_runner.h"

FileReader::FileReader(
    const extensions::ExtensionResource& resource,
    const OptionalFileSequenceTask& optional_file_sequence_task,
    const DoneCallback& done_callback)
    : resource_(resource),
      optional_file_sequence_task_(optional_file_sequence_task),
      done_callback_(done_callback),
      origin_task_runner_(base::ThreadTaskRunnerHandle::Get()) {}

void FileReader::Start() {
  extensions::GetExtensionFileTaskRunner()->PostTask(
      FROM_HERE, base::Bind(&FileReader::ReadFileOnFileSequence, this));
}

FileReader::~FileReader() {}

void FileReader::ReadFileOnFileSequence() {
  DCHECK(
      extensions::GetExtensionFileTaskRunner()->RunsTasksInCurrentSequence());

  std::unique_ptr<std::string> data(new std::string());
  bool success = base::ReadFileToString(resource_.GetFilePath(), data.get());

  if (!optional_file_sequence_task_.is_null()) {
    if (success) {
      base::ResetAndReturn(&optional_file_sequence_task_).Run(data.get());
    } else {
      optional_file_sequence_task_.Reset();
    }
  }

  origin_task_runner_->PostTask(
      FROM_HERE, base::Bind(base::ResetAndReturn(&done_callback_), success,
                            base::Passed(std::move(data))));
}

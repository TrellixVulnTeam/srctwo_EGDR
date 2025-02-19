// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/threading/thread_task_runner_handle.h"
#include "storage/browser/test/mock_file_update_observer.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace storage {

MockFileUpdateObserver::MockFileUpdateObserver() : is_ready_(false) {
}

MockFileUpdateObserver::~MockFileUpdateObserver() {
}

// static
UpdateObserverList MockFileUpdateObserver::CreateList(
    MockFileUpdateObserver* observer) {
  UpdateObserverList list;
  return list.AddObserver(observer, base::ThreadTaskRunnerHandle::Get().get());
}

void MockFileUpdateObserver::OnStartUpdate(const FileSystemURL& url) {
  if (is_ready_)
    ++start_update_count_[url];
}

void MockFileUpdateObserver::OnUpdate(const FileSystemURL& url, int64_t delta) {
  if (!is_ready_)
    return;
  int start = start_update_count_[url];
  int end = end_update_count_[url];
  EXPECT_LT(0, start - end);
}

void MockFileUpdateObserver::OnEndUpdate(const FileSystemURL& url) {
  if (is_ready_)
    ++end_update_count_[url];
}

}  // namespace storage

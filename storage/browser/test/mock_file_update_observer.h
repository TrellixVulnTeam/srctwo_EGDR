// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_FILEAPI_MOCK_FILE_UPDATE_OBSERVER_H_
#define CONTENT_BROWSER_FILEAPI_MOCK_FILE_UPDATE_OBSERVER_H_

#include <stdint.h>

#include <map>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "storage/browser/fileapi/file_observers.h"
#include "storage/browser/fileapi/file_system_url.h"
#include "storage/browser/fileapi/task_runner_bound_observer_list.h"

namespace storage {

// Mock file change observer for use in unittests.
class MockFileUpdateObserver : public FileUpdateObserver {
 public:
  MockFileUpdateObserver();
  ~MockFileUpdateObserver() override;

  // Creates a ChangeObserverList which only contains given |observer|.
  static UpdateObserverList CreateList(MockFileUpdateObserver* observer);

  // FileUpdateObserver overrides.
  void OnStartUpdate(const FileSystemURL& url) override;
  void OnUpdate(const FileSystemURL& url, int64_t delta) override;
  void OnEndUpdate(const FileSystemURL& url) override;

  void Enable() { is_ready_ = true; }

  void Disable() {
    start_update_count_.clear();
    end_update_count_.clear();
    is_ready_ = false;
  }

 private:
  std::map<FileSystemURL, int, FileSystemURL::Comparator> start_update_count_;
  std::map<FileSystemURL, int, FileSystemURL::Comparator> end_update_count_;
  bool is_ready_;

  DISALLOW_COPY_AND_ASSIGN(MockFileUpdateObserver);
};

}  // namespace storage

#endif  // CONTENT_BROWSER_FILEAPI_MOCK_FILE_UPDATE_OBSERVER_H_

// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_OFFLINE_PAGES_CORE_PREFETCH_DOWNLOAD_ARCHIVES_TASK_H_
#define COMPONENTS_OFFLINE_PAGES_CORE_PREFETCH_DOWNLOAD_ARCHIVES_TASK_H_

#include <memory>
#include <string>
#include <vector>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "components/offline_pages/core/task.h"

namespace offline_pages {
class PrefetchDownloader;
class PrefetchStore;

// Task that starts a download of archives for tasks that already received the
// bundle information.
class DownloadArchivesTask : public Task {
 public:
  // Maximum number of parallel downloads.
  static const int kMaxConcurrentDownloads;

  // Represents item to be downloaded as a result of running the task.
  struct DownloadItem {
    int64_t offline_id;
    std::string archive_body_name;
    int64_t archive_body_length;
    std::string guid;
  };

  using ItemsToDownload = std::vector<DownloadItem>;

  DownloadArchivesTask(PrefetchStore* prefetch_store,
                       PrefetchDownloader* prefetch_downloader);
  ~DownloadArchivesTask() override;

  void Run() override;

 private:
  void SendItemsToPrefetchDownloader(
      std::unique_ptr<ItemsToDownload> items_to_download);

  // Prefetch store to execute against. Not owned.
  PrefetchStore* prefetch_store_;
  // Prefetch downloader to request downloads from. Not owned.
  PrefetchDownloader* prefetch_downloader_;

  base::WeakPtrFactory<DownloadArchivesTask> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(DownloadArchivesTask);
};

}  // namespace offline_pages

#endif  // COMPONENTS_OFFLINE_PAGES_CORE_PREFETCH_DOWNLOAD_ARCHIVES_TASK_H_

// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_OFFLINE_PAGES_CORE_PREFETCH_PREFETCH_DOWNLOADER_IMPL_H_
#define COMPONENTS_OFFLINE_PAGES_CORE_PREFETCH_PREFETCH_DOWNLOADER_IMPL_H_

#include <string>
#include <utility>
#include <vector>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "components/download/public/download_params.h"
#include "components/offline_pages/core/prefetch/prefetch_downloader.h"
#include "components/offline_pages/core/prefetch/prefetch_types.h"
#include "components/version_info/channel.h"

namespace download {
class DownloadService;
}  // namespace download

namespace offline_pages {

class PrefetchService;
class PrefetchServiceTestTaco;

// Asynchronously downloads the archive.
class PrefetchDownloaderImpl : public PrefetchDownloader {
 public:
  PrefetchDownloaderImpl(download::DownloadService* download_service,
                         version_info::Channel channel);
  ~PrefetchDownloaderImpl() override;

  // PrefetchDownloader implementation:
  void SetPrefetchService(PrefetchService* service) override;
  bool IsDownloadServiceUnavailable() const override;
  void CleanupDownloadsWhenReady() override;
  void StartDownload(const std::string& download_id,
                     const std::string& download_location) override;
  void OnDownloadServiceReady(
      const std::set<std::string>& outstanding_download_ids,
      const std::map<std::string, std::pair<base::FilePath, int64_t>>&
          success_downloads) override;
  void OnDownloadServiceUnavailable() override;
  void OnDownloadSucceeded(const std::string& download_id,
                           const base::FilePath& file_path,
                           int64_t file_size) override;
  void OnDownloadFailed(const std::string& download_id) override;

 private:
  friend class PrefetchServiceTestTaco;

  enum class DownloadServiceStatus {
    // The download service is booting up.
    INITIALIZING,
    // The download service is up and can take downloads.
    STARTED,
    // The download service is unavailable to use for the whole lifetime of
    // Chrome.
    UNAVAILABLE,
  };

  // For test only.
  explicit PrefetchDownloaderImpl(version_info::Channel channel);

  // Callback for StartDownload.
  void OnStartDownload(const std::string& download_id,
                       download::DownloadParams::StartResult result);

  void CleanupDownloads(
      const std::set<std::string>& outstanding_download_ids,
      const std::map<std::string, std::pair<base::FilePath, int64_t>>&
          success_downloads);

  // Unowned. It is valid until |this| instance is disposed.
  download::DownloadService* download_service_;

  // Unowned, owns |this|.
  PrefetchService* prefetch_service_ = nullptr;

  version_info::Channel channel_;

  // Flag to indicate if the download service is ready to take downloads.
  DownloadServiceStatus download_service_status_ =
      DownloadServiceStatus::INITIALIZING;

  // Flag to indicate that the download cleanup can proceed.
  bool can_do_download_cleanup_ = false;

  std::set<std::string> outstanding_download_ids_;
  std::map<std::string, std::pair<base::FilePath, int64_t>> success_downloads_;

  base::WeakPtrFactory<PrefetchDownloaderImpl> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(PrefetchDownloaderImpl);
};

}  // namespace offline_pages

#endif  // COMPONENTS_OFFLINE_PAGES_CORE_PREFETCH_PREFETCH_DOWNLOADER_IMPL_H_

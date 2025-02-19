// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/offline_pages/core/prefetch/prefetch_downloader_impl.h"

#include <vector>

#include "components/download/internal/test/test_download_service.h"
#include "components/offline_pages/core/prefetch/prefetch_service.h"
#include "components/offline_pages/core/prefetch/prefetch_service_test_taco.h"
#include "components/offline_pages/core/prefetch/task_test_base.h"
#include "components/offline_pages/core/prefetch/test_download_client.h"
#include "components/offline_pages/core/prefetch/test_prefetch_dispatcher.h"
#include "net/base/url_util.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {
const version_info::Channel kTestChannel = version_info::Channel::UNKNOWN;
const char kDownloadId[] = "1234Ab";
const char kDownloadId2[] = "Abcd";
const char kFailedDownloadId[] = "f1f1FF";
const char kDownloadLocation[] = "page/1";
const char kDownloadLocation2[] = "page/zz";
const char kServerPathForDownload[] = "/v1/media/page/1";
}  // namespace

namespace offline_pages {

class PrefetchDownloaderImplTest : public TaskTestBase {
 public:
  PrefetchDownloaderImplTest() = default;

  void SetUp() override {
    prefetch_service_taco_.reset(new PrefetchServiceTestTaco);

    auto downloader = base::MakeUnique<PrefetchDownloaderImpl>(
        &download_service_, kTestChannel);
    download_service_.SetFailedDownload(kFailedDownloadId, false);
    download_service_.SetIsReady(true);
    download_client_ = base::MakeUnique<TestDownloadClient>(downloader.get());
    download_service_.set_client(download_client_.get());
    prefetch_service_taco_->SetPrefetchDownloader(std::move(downloader));
    prefetch_service_taco_->CreatePrefetchService();

    prefetch_downloader()->OnDownloadServiceReady(
        std::set<std::string>(),
        std::map<std::string, std::pair<base::FilePath, int64_t>>());
  }

  void TearDown() override {
    prefetch_service_taco_.reset();
    TaskTestBase::TearDown();
  }

  void StartDownload(const std::string& download_id,
                     const std::string& download_location) {
    prefetch_downloader()->StartDownload(download_id, download_location);
  }

  base::Optional<download::DownloadParams> GetDownload(
      const std::string& guid) const {
    return download_service_.GetDownload(guid);
  }

  const std::vector<PrefetchDownloadResult>& completed_downloads() const {
    return prefetch_dispatcher()->download_results;
  }

 private:
  PrefetchDownloader* prefetch_downloader() const {
    return prefetch_service_taco_->prefetch_service()->GetPrefetchDownloader();
  }

  TestPrefetchDispatcher* prefetch_dispatcher() const {
    return static_cast<TestPrefetchDispatcher*>(
        prefetch_service_taco_->prefetch_service()->GetPrefetchDispatcher());
  }

  download::test::TestDownloadService download_service_;
  std::unique_ptr<TestDownloadClient> download_client_;
  std::unique_ptr<PrefetchServiceTestTaco> prefetch_service_taco_;
};

TEST_F(PrefetchDownloaderImplTest, DownloadParams) {
  StartDownload(kDownloadId, kDownloadLocation);
  base::Optional<download::DownloadParams> params = GetDownload(kDownloadId);
  ASSERT_TRUE(params.has_value());
  EXPECT_EQ(kDownloadId, params->guid);
  EXPECT_EQ(download::DownloadClient::OFFLINE_PAGE_PREFETCH, params->client);
  GURL download_url = params->request_params.url;
  EXPECT_TRUE(download_url.SchemeIs(url::kHttpsScheme));
  EXPECT_EQ(kServerPathForDownload, download_url.path());
  std::string key_value;
  EXPECT_TRUE(net::GetValueForKeyInQuery(download_url, "key", &key_value));
  EXPECT_FALSE(key_value.empty());
  std::string alt_value;
  EXPECT_TRUE(net::GetValueForKeyInQuery(download_url, "alt", &alt_value));
  EXPECT_EQ("media", alt_value);
  RunUntilIdle();
}

TEST_F(PrefetchDownloaderImplTest, DownloadSucceeded) {
  StartDownload(kDownloadId, kDownloadLocation);
  StartDownload(kDownloadId2, kDownloadLocation2);
  RunUntilIdle();
  ASSERT_EQ(2u, completed_downloads().size());
  EXPECT_EQ(kDownloadId, completed_downloads()[0].download_id);
  EXPECT_TRUE(completed_downloads()[0].success);
  EXPECT_EQ(kDownloadId2, completed_downloads()[1].download_id);
  EXPECT_TRUE(completed_downloads()[1].success);
}

TEST_F(PrefetchDownloaderImplTest, DownloadFailed) {
  StartDownload(kFailedDownloadId, kDownloadLocation);
  RunUntilIdle();
  ASSERT_EQ(1u, completed_downloads().size());
  EXPECT_EQ(kFailedDownloadId, completed_downloads()[0].download_id);
  EXPECT_FALSE(completed_downloads()[0].success);
}

}  // namespace offline_pages

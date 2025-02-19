// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/chrome/browser/reading_list/url_downloader.h"

#include <vector>

#include "base/files/file_util.h"
#import "base/mac/bind_objc_block.h"
#include "base/path_service.h"
#include "base/stl_util.h"
#import "base/test/ios/wait_util.h"
#include "base/test/scoped_task_environment.h"
#include "components/reading_list/core/offline_url_utils.h"
#include "ios/chrome/browser/chrome_paths.h"
#include "ios/chrome/browser/dom_distiller/distiller_viewer.h"
#include "ios/chrome/browser/reading_list/offline_url_utils.h"
#include "ios/chrome/browser/reading_list/reading_list_distiller_page.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

class DistillerViewerTest : public dom_distiller::DistillerViewerInterface {
 public:
  DistillerViewerTest(const GURL& url,
                      const DistillationFinishedCallback& callback,
                      reading_list::ReadingListDistillerPageDelegate* delegate,
                      const std::string& html,
                      const GURL& redirect_url,
                      const std::string& mime_type)
      : dom_distiller::DistillerViewerInterface(nil) {
    std::vector<ImageInfo> images;
    ImageInfo image;
    image.url = GURL("http://image");
    image.data = "image";
    images.push_back(image);

    if (redirect_url.is_valid()) {
      delegate->DistilledPageRedirectedToURL(url, redirect_url);
    }
    if (!mime_type.empty()) {
      delegate->DistilledPageHasMimeType(url, mime_type);
    }
    callback.Run(url, html, images, "title");
  }

  void OnArticleReady(
      const dom_distiller::DistilledArticleProto* article_proto) override {}

  void SendJavaScript(const std::string& buffer) override {}
};

void RemoveOfflineFilesDirectory(base::FilePath base_directory) {
  base::DeleteFile(reading_list::OfflineRootDirectoryPath(base_directory),
                   true);
}

}  // namespace

class MockURLDownloader : public URLDownloader {
 public:
  MockURLDownloader(base::FilePath path)
      : URLDownloader(nullptr,
                      nullptr,
                      nullptr,
                      path,
                      nullptr,
                      base::Bind(&MockURLDownloader::OnEndDownload,
                                 base::Unretained(this)),
                      base::Bind(&MockURLDownloader::OnEndRemove,
                                 base::Unretained(this))),
        html_("html") {}

  void ClearCompletionTrackers() {
    downloaded_files_.clear();
    removed_files_.clear();
  }

  bool CheckExistenceOfOfflineURLPagePath(const GURL& url) {
    return base::PathExists(
        reading_list::OfflineURLAbsolutePathFromRelativePath(
            base_directory_, reading_list::OfflinePagePath(
                                 url, reading_list::OFFLINE_TYPE_HTML)));
  }

  void FakeWorking() { working_ = true; }

  void FakeEndWorking() {
    working_ = false;
    HandleNextTask();
  }

  std::vector<GURL> downloaded_files_;
  std::vector<GURL> removed_files_;
  GURL redirect_url_;
  std::string mime_type_;
  std::string html_;
  bool fetching_pdf_;

 private:
  void DownloadURL(const GURL& url, bool offline_url_exists) override {
    if (offline_url_exists) {
      DownloadCompletionHandler(url, std::string(), base::FilePath(),
                                DOWNLOAD_EXISTS);
      return;
    }
    original_url_ = url;
    saved_size_ = 0;
    distiller_.reset(new DistillerViewerTest(
        url,
        base::Bind(&URLDownloader::DistillerCallback, base::Unretained(this)),
        this, html_, redirect_url_, mime_type_));
  }

  void FetchPDFFile() override { fetching_pdf_ = true; }

  void OnEndDownload(const GURL& url,
                     const GURL& distilled_url,
                     SuccessState success,
                     const base::FilePath& distilled_path,
                     int64_t size,
                     const std::string& title) {
    downloaded_files_.push_back(url);
    // Saved data is the string "html" and an image with data "image".
    EXPECT_EQ(size, 9);
    EXPECT_EQ(distilled_url, redirect_url_);
  }

  void OnEndRemove(const GURL& url, bool success) {
    removed_files_.push_back(url);
  }
};

namespace {
class URLDownloaderTest : public testing::Test {
 public:
  URLDownloaderTest() {
    base::FilePath data_dir;
    base::PathService::Get(ios::DIR_USER_DATA, &data_dir);
    RemoveOfflineFilesDirectory(data_dir);
    downloader_.reset(new MockURLDownloader(data_dir));
  }

  ~URLDownloaderTest() override {}

  void TearDown() override {
    base::FilePath data_dir;
    base::PathService::Get(ios::DIR_USER_DATA, &data_dir);
    RemoveOfflineFilesDirectory(data_dir);
    downloader_->ClearCompletionTrackers();
  }

 protected:
  base::test::ScopedTaskEnvironment task_environment_;
  std::unique_ptr<MockURLDownloader> downloader_;

 private:
  DISALLOW_COPY_AND_ASSIGN(URLDownloaderTest);
};

TEST_F(URLDownloaderTest, SingleDownload) {
  GURL url = GURL("http://test.com");
  ASSERT_FALSE(downloader_->CheckExistenceOfOfflineURLPagePath(url));
  ASSERT_EQ(0ul, downloader_->downloaded_files_.size());
  ASSERT_EQ(0ul, downloader_->removed_files_.size());

  downloader_->DownloadOfflineURL(url);

  // Wait for all asynchronous tasks to complete.
  task_environment_.RunUntilIdle();

  ASSERT_TRUE(downloader_->CheckExistenceOfOfflineURLPagePath(url));
}

TEST_F(URLDownloaderTest, SingleDownloadRedirect) {
  GURL url = GURL("http://test.com");
  ASSERT_FALSE(downloader_->CheckExistenceOfOfflineURLPagePath(url));
  ASSERT_EQ(0ul, downloader_->downloaded_files_.size());
  ASSERT_EQ(0ul, downloader_->removed_files_.size());

  // The DCHECK in OnEndDownload will verify that the redirection was handled
  // correctly.
  downloader_->redirect_url_ = GURL("http://test.com/redirected");

  downloader_->DownloadOfflineURL(url);

  // Wait for all asynchronous tasks to complete.
  task_environment_.RunUntilIdle();

  EXPECT_TRUE(downloader_->CheckExistenceOfOfflineURLPagePath(url));
}

TEST_F(URLDownloaderTest, SingleDownloadPDF) {
  GURL url = GURL("http://test.com");
  ASSERT_FALSE(downloader_->CheckExistenceOfOfflineURLPagePath(url));
  ASSERT_EQ(0ul, downloader_->downloaded_files_.size());
  ASSERT_EQ(0ul, downloader_->removed_files_.size());

  downloader_->mime_type_ = "application/pdf";
  downloader_->html_ = "";

  downloader_->DownloadOfflineURL(url);

  // Wait for all asynchronous tasks to complete.
  task_environment_.RunUntilIdle();

  EXPECT_FALSE(downloader_->CheckExistenceOfOfflineURLPagePath(url));
}

TEST_F(URLDownloaderTest, DownloadAndRemove) {
  GURL url = GURL("http://test.com");
  GURL url2 = GURL("http://test2.com");
  ASSERT_FALSE(downloader_->CheckExistenceOfOfflineURLPagePath(url));
  ASSERT_FALSE(downloader_->CheckExistenceOfOfflineURLPagePath(url2));
  ASSERT_EQ(0ul, downloader_->downloaded_files_.size());
  ASSERT_EQ(0ul, downloader_->removed_files_.size());
  downloader_->FakeWorking();
  downloader_->DownloadOfflineURL(url);
  downloader_->DownloadOfflineURL(url2);
  downloader_->RemoveOfflineURL(url);
  downloader_->FakeEndWorking();

  // Wait for all asynchronous tasks to complete.
  task_environment_.RunUntilIdle();

  ASSERT_TRUE(!base::ContainsValue(downloader_->downloaded_files_, url));
  ASSERT_EQ(1ul, downloader_->downloaded_files_.size());
  ASSERT_EQ(1ul, downloader_->removed_files_.size());
  ASSERT_FALSE(downloader_->CheckExistenceOfOfflineURLPagePath(url));
  ASSERT_TRUE(downloader_->CheckExistenceOfOfflineURLPagePath(url2));
}

TEST_F(URLDownloaderTest, DownloadAndRemoveAndRedownload) {
  GURL url = GURL("http://test.com");
  ASSERT_FALSE(downloader_->CheckExistenceOfOfflineURLPagePath(url));
  downloader_->FakeWorking();
  downloader_->DownloadOfflineURL(url);
  downloader_->RemoveOfflineURL(url);
  downloader_->DownloadOfflineURL(url);
  downloader_->FakeEndWorking();

  // Wait for all asynchronous tasks to complete.
  task_environment_.RunUntilIdle();

  ASSERT_TRUE(base::ContainsValue(downloader_->downloaded_files_, url));
  ASSERT_TRUE(base::ContainsValue(downloader_->removed_files_, url));
  ASSERT_TRUE(downloader_->CheckExistenceOfOfflineURLPagePath(url));
}

}  // namespace

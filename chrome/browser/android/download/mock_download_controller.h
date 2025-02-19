// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ANDROID_DOWNLOAD_MOCK_DOWNLOAD_CONTROLLER_H_
#define CHROME_BROWSER_ANDROID_DOWNLOAD_MOCK_DOWNLOAD_CONTROLLER_H_

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/singleton.h"
#include "chrome/browser/android/download/download_controller_base.h"

namespace content {
class DownloadItem;
class WebContents;
}  // namespace content

namespace chrome {
namespace android {

// Mock implementation of the DownloadController.
class MockDownloadController : public DownloadControllerBase {
 public:
  MockDownloadController();
  ~MockDownloadController() override;

  // DownloadControllerBase implementation.
  void OnDownloadStarted(content::DownloadItem* download_item) override;
  void StartContextMenuDownload(
      const content::ContextMenuParams& params,
      content::WebContents* web_contents,
      bool is_link, const std::string& extra_headers) override;
  void AcquireFileAccessPermission(
      const content::ResourceRequestInfo::WebContentsGetter& wc_getter,
      const AcquireFileAccessPermissionCallback& callback) override;
  void SetApproveFileAccessRequestForTesting(bool approve) override;
  void CreateAndroidDownload(
      const content::ResourceRequestInfo::WebContentsGetter& wc_getter,
      const DownloadInfo& info) override;
  void AboutToResumeDownload(content::DownloadItem* download_item) override;

 private:
  bool approve_file_access_request_;
  DISALLOW_COPY_AND_ASSIGN(MockDownloadController);
};

}  // namespace android
}  // namespace chrome


#endif  // CHROME_BROWSER_ANDROID_DOWNLOAD_MOCK_DOWNLOAD_CONTROLLER_H_

// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_DOWNLOAD_INTERNAL_TEST_EMPTY_CLIENT_H_
#define COMPONENTS_DOWNLOAD_INTERNAL_TEST_EMPTY_CLIENT_H_

#include "base/macros.h"
#include "components/download/public/client.h"
#include "components/download/public/download_metadata.h"

namespace download {
namespace test {

class EmptyClient : public Client {
 public:
  EmptyClient() = default;
  ~EmptyClient() override = default;

  // Client implementation.
  void OnServiceInitialized(
      bool state_lost,
      const std::vector<DownloadMetaData>& downloads) override;
  void OnServiceUnavailable() override;
  ShouldDownload OnDownloadStarted(
      const std::string& guid,
      const std::vector<GURL>& url_chain,
      const scoped_refptr<const net::HttpResponseHeaders>& headers) override;
  void OnDownloadUpdated(const std::string& guid,
                         uint64_t bytes_downloaded) override;
  void OnDownloadFailed(const std::string& guid, FailureReason reason) override;
  void OnDownloadSucceeded(const std::string& guid,
                           const CompletionInfo& completion_info) override;
  bool CanServiceRemoveDownloadedFile(const std::string& guid,
                                      bool force_delete) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(EmptyClient);
};

}  // namespace test
}  // namespace download

#endif  // COMPONENTS_DOWNLOAD_INTERNAL_TEST_EMPTY_CLIENT_H_

// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/download/internal/model.h"

#include "components/download/internal/entry.h"
#include "testing/gmock/include/gmock/gmock.h"

#ifndef COMPONENTS_DOWNLOAD_INTERNAL_TEST_MOCK_MODEL_CLIENT_H_
#define COMPONENTS_DOWNLOAD_INTERNAL_TEST_MOCK_MODEL_CLIENT_H_

namespace download {
namespace test {

class MockModelClient : public Model::Client {
 public:
  MockModelClient();
  ~MockModelClient() override;

  // Model::Client implementation.
  MOCK_METHOD1(OnModelReady, void(bool));
  MOCK_METHOD1(OnModelHardRecoverComplete, void(bool));
  MOCK_METHOD3(OnItemAdded, void(bool, DownloadClient, const std::string&));
  MOCK_METHOD3(OnItemUpdated, void(bool, DownloadClient, const std::string&));
  MOCK_METHOD3(OnItemRemoved, void(bool, DownloadClient, const std::string&));
};

}  // namespace test
}  // namespace download

#endif  // COMPONENTS_DOWNLOAD_INTERNAL_TEST_MOCK_MODEL_CLIENT_H_

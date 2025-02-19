// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/policy/core/common/cloud/mock_cloud_external_data_manager.h"

#include "base/callback.h"
#include "base/memory/ptr_util.h"
#include "base/memory/weak_ptr.h"
#include "components/policy/core/common/external_data_fetcher.h"
#include "net/url_request/url_request_context_getter.h"

namespace policy {

MockCloudExternalDataManager::MockCloudExternalDataManager() {
}

MockCloudExternalDataManager::~MockCloudExternalDataManager() {
}

std::unique_ptr<ExternalDataFetcher>
MockCloudExternalDataManager::CreateExternalDataFetcher(
    const std::string& policy) {
  return base::MakeUnique<ExternalDataFetcher>(weak_factory_.GetWeakPtr(),
                                               policy);
}

}  // namespace policy

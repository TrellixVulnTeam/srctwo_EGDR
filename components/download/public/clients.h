// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_DOWNLOAD_PUBLIC_CLIENTS_H_
#define COMPONENTS_DOWNLOAD_PUBLIC_CLIENTS_H_

#include <map>
#include <memory>

#include "components/download/public/client.h"

namespace download {

// A list of all clients that are able to make download requests through the
// DownloadService.
// To add a new client, update the metric DownloadService.DownloadClients in
// histograms.xml and make sure to keep this list in sync.  Additions should be
// treated as APPEND ONLY to make sure to keep both UMA metric semantics correct
// but also to make sure the underlying database properly associates each
// download with the right client.
enum class DownloadClient {
  // Test client values.  Meant to be used by the testing framework and not
  // production code.  Callers will be unable to access the DownloadService with
  // these test APIs.
  TEST = -1,
  TEST_2 = -2,
  TEST_3 = -3,

  // Represents an uninitialized DownloadClient variable.
  INVALID = 0,

  OFFLINE_PAGE_PREFETCH = 1,

  BOUNDARY = 2,
};

using DownloadClientMap = std::map<DownloadClient, std::unique_ptr<Client>>;

}  // namespace download

#endif  // COMPONENTS_DOWNLOAD_PUBLIC_CLIENTS_H_

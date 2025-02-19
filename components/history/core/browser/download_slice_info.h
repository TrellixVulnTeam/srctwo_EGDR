// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_HISTORY_CORE_BROWSER_DOWNLOAD_SLICE_INFO_H_
#define COMPONENTS_HISTORY_CORE_BROWSER_DOWNLOAD_SLICE_INFO_H_

#include <stdint.h>
#include <string>

#include "components/history/core/browser/download_types.h"

namespace history {

// Contains the information for each slice of data that is written to the
// download target file. A download file can have multiple slices and cach
// slice will have a different offset.
struct DownloadSliceInfo {
  DownloadSliceInfo();
  DownloadSliceInfo(DownloadId download_id,
                    int64_t offset,
                    int64_t received_bytes);
  DownloadSliceInfo(const DownloadSliceInfo& other);
  ~DownloadSliceInfo();

  bool operator==(const DownloadSliceInfo&) const;

  // The id of the download in the database.
  DownloadId download_id;

  // Start position of the download request.
  int64_t offset;

  // The number of bytes received (so far).
  int64_t received_bytes;
};

}  // namespace history

#endif  // COMPONENTS_HISTORY_CORE_BROWSER_DOWNLOAD_SLICE_INFO_H_

// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/history/content/browser/history_database_helper.h"

#include "base/files/file_path.h"
#include "components/history/content/browser/download_conversions.h"
#include "components/history/core/browser/history_database_params.h"
#include "content/public/browser/download_interrupt_reasons.h"

namespace history {

HistoryDatabaseParams HistoryDatabaseParamsForPath(
    const base::FilePath& history_dir) {
  return HistoryDatabaseParams(
      history_dir,
      history::ToHistoryDownloadInterruptReason(
          content::DOWNLOAD_INTERRUPT_REASON_NONE),
      history::ToHistoryDownloadInterruptReason(
          content::DOWNLOAD_INTERRUPT_REASON_CRASH));
}

}  // namespace

// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_DOWNLOAD_INTERNAL_CONFIG_H_
#define COMPONENTS_DOWNLOAD_INTERNAL_CONFIG_H_

#include <memory>

#include "base/macros.h"
#include "base/time/time.h"

namespace download {

// Configuration name for max concurrent downloads.
constexpr char kMaxConcurrentDownloadsConfig[] = "max_concurrent_downloads";

// Configuration name for maximum running downloads.
constexpr char kMaxRunningDownloadsConfig[] = "max_running_downloads";

// Configuration name for maximum scheduled downloads.
constexpr char kMaxScheduledDownloadsConfig[] = "max_scheduled_downloads";

// Configuration name for maximum retry count.
constexpr char kMaxRetryCountConfig[] = "max_retry_count";

// Configuration name for file keep alive time.
constexpr char kFileKeepAliveTimeMinutesConfig[] =
    "file_keep_alive_time_minutes";

// Configuration name for maximum duration that the file can be kept alive.
constexpr char kMaxFileKeepAliveTimeMinutesConfig[] =
    "max_file_keep_alive_time_minutes";

// Configuration name for file keep alive time.
constexpr char kFileCleanupWindowMinutesConfig[] = "file_cleanup_window";

// Configuration name for window start time.
constexpr char kWindowStartTimeSecondsConfig[] = "window_start_time_seconds";

// Configuration name for window end time.
constexpr char kWindowEndTimeSecondsConfig[] = "window_end_time_seconds";

// Configuration name for the delay to notify network status change, measured in
// milliseconds.
constexpr char kNetworkChangeDelayMsConfig[] = "network_change_delay_ms";

// Configuration name for the retry delay when the download is failed, measured
// in milliseconds.
constexpr char kDownloadRetryDelayMsConfig[] = "retry_delay_ms";

// Download service configuration.
//
// Loaded based on experiment parameters from the server. Use default values if
// no server configuration was detected.
struct Configuration {
 public:
  // Create the configuration.
  static std::unique_ptr<Configuration> CreateFromFinch();
  Configuration();

  // The maximum number of downloads the DownloadService can have currently in
  // Active or Paused states.
  uint32_t max_concurrent_downloads;

  // The maximum number of downloads the DownloadService can have currently in
  // only Active state.
  uint32_t max_running_downloads;

  // The maximum number of downloads that are scheduled for each client using
  // the download service.
  uint32_t max_scheduled_downloads;

  // The maximum number of retries before the download is aborted.
  uint32_t max_retry_count;

  // The time that the download service will keep the files around before
  // deleting them if the client hasn't handle the files.
  base::TimeDelta file_keep_alive_time;

  // The maximum time that the download service can keep the files around before
  // forcefully deleting them even if the client doesn't agree.
  base::TimeDelta max_file_keep_alive_time;

  // The length of the flexible time window during which the scheduler must
  // schedule a file cleanup task.
  base::TimeDelta file_cleanup_window;

  // The start window time in seconds for OS to schedule background task.
  // The OS will trigger the background task in this window.
  base::TimeDelta window_start_time;

  // The end window time in seconds for OS to schedule background task.
  // The OS will trigger the background task in this window.
  base::TimeDelta window_end_time;

  // The delay to notify network status changes.
  base::TimeDelta network_change_delay;

  // The delay to retry a download when the download is failed.
  base::TimeDelta download_retry_delay;

 private:
  DISALLOW_COPY_AND_ASSIGN(Configuration);
};

}  // namespace download

#endif  // COMPONENTS_DOWNLOAD_INTERNAL_CONFIG_H_

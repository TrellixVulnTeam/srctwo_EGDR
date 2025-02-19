// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_CRASH_LINUX_MINIDUMP_UPLOADER_H_
#define CHROMECAST_CRASH_LINUX_MINIDUMP_UPLOADER_H_

#include <string>

#include "base/callback.h"
#include "base/macros.h"
#include "chromecast/crash/linux/synchronized_minidump_manager.h"

class PrefService;

namespace chromecast {

class CastCrashdumpUploader;
class CastSysInfo;

// Class for uploading minidumps with synchronized access to the minidumps
// directory.
class MinidumpUploader : public SynchronizedMinidumpManager {
 public:
  using PrefServiceGeneratorCallback =
      base::Callback<std::unique_ptr<PrefService>()>;

  // If |server_url| is empty, a default server url will be chosen.
  MinidumpUploader(CastSysInfo* sys_info, const std::string& server_url);
  MinidumpUploader(CastSysInfo* sys_info,
                   const std::string& server_url,
                   CastCrashdumpUploader* const uploader,
                   const PrefServiceGeneratorCallback callback);
  ~MinidumpUploader() override;

  // Attempts to upload all minidumps in the minidumps directory. Acquires a
  // mutually exclusive lock on the directory before doing work to ensure that
  // access to these minidumps is synchronized between other instances of this
  // class. Returns true if successful, false otherwise.
  bool UploadAllMinidumps();

  bool reboot_scheduled() const { return reboot_scheduled_; }

 private:
  // SynchronizedMinidumpManager implementation:
  bool DoWork() override;

  // From CastSysInfo.
  const std::string release_channel_;
  const std::string product_name_;
  const std::string device_model_;
  const std::string board_name_;
  const std::string board_revision_;
  const std::string manufacturer_;
  const std::string system_version_;

  const std::string upload_location_;

  // Whether or not we were ratelimited for the last upload.
  // Used to detect when we first become ratelimited. Must be initialized to
  // true to prevent reboot loops when ratelimited.
  bool last_upload_ratelimited_;

  // Whether or not a reboot should be scheduled.
  bool reboot_scheduled_;

  // True if file state has been initialized.
  bool filestate_initialized_;

  // Used for injecting mocks/inducing different behavior in unittests.
  CastCrashdumpUploader* const uploader_;
  const PrefServiceGeneratorCallback pref_service_generator_;

  DISALLOW_COPY_AND_ASSIGN(MinidumpUploader);
};

}  // namespace chromecast

#endif  // CHROMECAST_CRASH_LINUX_MINIDUMP_UPLOADER_H_

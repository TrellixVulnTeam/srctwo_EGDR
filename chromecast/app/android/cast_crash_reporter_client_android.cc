// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromecast/app/android/cast_crash_reporter_client_android.h"

#include "base/base_paths.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "chromecast/base/chromecast_config_android.h"
#include "chromecast/base/version.h"
#include "chromecast/common/global_descriptors.h"
#include "chromecast/crash/cast_crash_keys.h"
#include "content/public/common/content_switches.h"

namespace chromecast {

CastCrashReporterClientAndroid::CastCrashReporterClientAndroid(
    const std::string& process_type)
    : process_type_(process_type) {
}

CastCrashReporterClientAndroid::~CastCrashReporterClientAndroid() {
}

void CastCrashReporterClientAndroid::GetProductNameAndVersion(
    const char** product_name,
    const char** version) {
  *product_name = "media_shell";
  *version = PRODUCT_VERSION
#if CAST_IS_DEBUG_BUILD()
      ".debug"
#endif
      "." CAST_BUILD_REVISION;
}

base::FilePath CastCrashReporterClientAndroid::GetReporterLogFilename() {
  return base::FilePath(FILE_PATH_LITERAL("uploads.log"));
}

// static
bool CastCrashReporterClientAndroid::GetCrashDumpLocation(
    const std::string& process_type,
    base::FilePath* crash_dir) {
  base::FilePath crash_dir_local;
  if (!PathService::Get(base::DIR_ANDROID_APP_DATA, &crash_dir_local)) {
    return false;
  }
  crash_dir_local = crash_dir_local.Append("crashes");

  // Only try to create the directory in the browser process (empty value).
  if (process_type.empty()) {
    if (!base::DirectoryExists(crash_dir_local)) {
      if (!base::CreateDirectory(crash_dir_local)) {
        return false;
      }
    }
  }

  // Provide value to crash_dir once directory is known to be a valid path.
  *crash_dir = crash_dir_local;
  return true;
}

bool CastCrashReporterClientAndroid::GetCrashDumpLocation(
    base::FilePath* crash_dir) {
  return CastCrashReporterClientAndroid::GetCrashDumpLocation(process_type_,
                                                              crash_dir);
}

size_t CastCrashReporterClientAndroid::RegisterCrashKeys() {
  return crash_keys::RegisterCastCrashKeys();
}

bool CastCrashReporterClientAndroid::GetCollectStatsConsent() {
  return android::ChromecastConfigAndroid::GetInstance()->CanSendUsageStats();
}

int CastCrashReporterClientAndroid::GetAndroidMinidumpDescriptor() {
  return kAndroidMinidumpDescriptor;
}

bool CastCrashReporterClientAndroid::EnableBreakpadForProcess(
    const std::string& process_type) {
  return process_type == switches::kRendererProcess ||
         process_type == switches::kGpuProcess;
}

}  // namespace chromecast

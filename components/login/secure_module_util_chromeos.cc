// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/login/secure_module_util_chromeos.h"

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/task_scheduler/post_task.h"

namespace login {

namespace {

// If either of these two files exist, then we are using Cr50 and not using TPM.
constexpr char kCr50UsedIndicatorFile1[] =
    "/opt/google/cr50/firmware/cr50.bin.prod";
constexpr char kCr50UsedIndicatorFile2[] = "/etc/init/cr50-update.conf";

SecureModuleUsed g_secure_module_used = SecureModuleUsed::UNQUERIED;

SecureModuleUsed GetSecureModuleInfoFromFilesAndCacheIt() {
  if (base::PathExists(base::FilePath(kCr50UsedIndicatorFile1)) ||
      base::PathExists(base::FilePath(kCr50UsedIndicatorFile2))) {
    g_secure_module_used = SecureModuleUsed::CR50;
  } else {
    g_secure_module_used = SecureModuleUsed::TPM;
  }
  return g_secure_module_used;
}

}  // namespace

void GetSecureModuleUsed(GetSecureModuleUsedCallback callback) {
  if (g_secure_module_used != SecureModuleUsed::UNQUERIED) {
    base::ResetAndReturn(&callback).Run(g_secure_module_used);
    return;
  }

  base::PostTaskWithTraitsAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::BACKGROUND},
      base::BindOnce(&GetSecureModuleInfoFromFilesAndCacheIt),
      std::move(callback));
}

}  // namespace login

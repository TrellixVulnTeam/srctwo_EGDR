// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/policy/remote_commands/device_command_fetch_status_job.h"

#include <memory>
#include <utility>

#include "base/bind.h"
#include "base/syslog_logging.h"
#include "base/threading/thread_task_runner_handle.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/chromeos/policy/browser_policy_connector_chromeos.h"
#include "chrome/browser/chromeos/policy/device_cloud_policy_manager_chromeos.h"
#include "chrome/browser/chromeos/policy/status_uploader.h"
#include "chrome/browser/chromeos/policy/system_log_uploader.h"
#include "components/policy/proto/device_management_backend.pb.h"

namespace policy {

namespace {

// Determines the time, measured from the time of issue, after which the command
// queue will consider this command expired if the command has not been started.
const int kCommandExpirationTimeInMinutes = 10;

}  // namespace

DeviceCommandFetchStatusJob::DeviceCommandFetchStatusJob() {}

DeviceCommandFetchStatusJob::~DeviceCommandFetchStatusJob() {}

enterprise_management::RemoteCommand_Type
DeviceCommandFetchStatusJob::GetType() const {
  return enterprise_management::RemoteCommand_Type_DEVICE_FETCH_STATUS;
}

base::TimeDelta DeviceCommandFetchStatusJob::GetCommmandTimeout() const {
  return base::TimeDelta::FromMinutes(kCommandExpirationTimeInMinutes);
}

bool DeviceCommandFetchStatusJob::IsExpired(base::TimeTicks now) {
  return now > issued_time() + GetCommmandTimeout();
}

void DeviceCommandFetchStatusJob::RunImpl(
    const CallbackWithResult& succeeded_callback,
    const CallbackWithResult& failed_callback) {
  SYSLOG(INFO) << "Fetching device status";
  BrowserPolicyConnectorChromeOS* connector =
      g_browser_process->platform_part()->browser_policy_connector_chromeos();
  DeviceCloudPolicyManagerChromeOS* manager =
      connector->GetDeviceCloudPolicyManager();
  // DeviceCloudPolicyManagerChromeOS, StatusUploader and SystemLogUploader can
  // be null during shutdown (and unit tests). StatusUploader and
  // SystemLogUploader objects have the same lifetime - they are created
  // together and they are destroyed together (which is why this code doesn't
  // do separate checks for them before using them).
  if (manager && manager->GetStatusUploader() &&
      manager->GetSystemLogUploader()) {
    manager->GetStatusUploader()->ScheduleNextStatusUploadImmediately();
    manager->GetSystemLogUploader()->ScheduleNextSystemLogUploadImmediately();
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::BindOnce(succeeded_callback, nullptr));
    return;
  }

  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE, base::BindOnce(failed_callback, nullptr));
}

}  // namespace policy

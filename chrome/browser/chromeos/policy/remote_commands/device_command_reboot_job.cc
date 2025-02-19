// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/policy/remote_commands/device_command_reboot_job.h"

#include <algorithm>

#include "base/bind.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/single_thread_task_runner.h"
#include "base/sys_info.h"
#include "base/syslog_logging.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/time/time.h"
#include "chromeos/dbus/power_manager_client.h"
#include "components/policy/proto/device_management_backend.pb.h"

namespace policy {

namespace {

// Determines the time, measured from the time of issue, after which the command
// queue will consider this command expired if the command has not been started.
const int kCommandExpirationTimeInMinutes = 10;

}  // namespace

DeviceCommandRebootJob::DeviceCommandRebootJob(
    chromeos::PowerManagerClient* power_manager_client)
    : power_manager_client_(power_manager_client) {
  CHECK(power_manager_client_);
}

DeviceCommandRebootJob::~DeviceCommandRebootJob() {
}

enterprise_management::RemoteCommand_Type DeviceCommandRebootJob::GetType()
    const {
  return enterprise_management::RemoteCommand_Type_DEVICE_REBOOT;
}

bool DeviceCommandRebootJob::IsExpired(base::TimeTicks now) {
  return now > issued_time() + base::TimeDelta::FromMinutes(
                                   kCommandExpirationTimeInMinutes);
}

void DeviceCommandRebootJob::RunImpl(
    const CallbackWithResult& succeeded_callback,
    const CallbackWithResult& failed_callback) {
  SYSLOG(INFO) << "Running reboot command.";

  // Determines the time delta between the command having been issued and the
  // boot time of the system.
  const base::TimeDelta uptime = base::SysInfo::Uptime();
  const base::TimeTicks boot_time = base::TimeTicks::Now() - uptime;
  const base::TimeDelta delta = boot_time - issued_time();
  // If the reboot command was issued before the system booted, we inform the
  // server that the reboot succeeded. Otherwise, the reboot must still be
  // performed and we invoke it.
  if (delta > base::TimeDelta()) {
    SYSLOG(WARNING) << "Ignoring reboot command issued " << delta
                    << " before current boot time";
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::BindOnce(succeeded_callback, nullptr));
    return;
  }

  SYSLOG(INFO) << "Rebooting immediately.";
  power_manager_client_->RequestRestart();
}

base::TimeDelta DeviceCommandRebootJob::GetCommmandTimeout() const {
  return base::TimeDelta::FromMinutes(kCommandExpirationTimeInMinutes);
}

}  // namespace policy

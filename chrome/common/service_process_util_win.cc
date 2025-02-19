// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/common/service_process_util.h"

#include <algorithm>
#include <memory>

#include "base/callback.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/object_watcher.h"
#include "base/win/scoped_handle.h"
#include "base/win/win_util.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/common/chrome_switches.h"

namespace {

const char kTerminateEventSuffix[] = "_service_terminate_evt";

base::string16 GetServiceProcessReadyEventName() {
  return base::UTF8ToWide(
      GetServiceProcessScopedVersionedName("_service_ready"));
}

base::string16 GetServiceProcessTerminateEventName() {
  return base::UTF8ToWide(
      GetServiceProcessScopedVersionedName(kTerminateEventSuffix));
}

std::string GetServiceProcessAutoRunKey() {
  return GetServiceProcessScopedName("_service_run");
}

// Returns the name of the autotun reg value that we used to use for older
// versions of Chrome.
std::string GetObsoleteServiceProcessAutoRunKey() {
  base::FilePath user_data_dir;
  PathService::Get(chrome::DIR_USER_DATA, &user_data_dir);
  std::string scoped_name = base::WideToUTF8(user_data_dir.value());
  std::replace(scoped_name.begin(), scoped_name.end(), '\\', '!');
  std::replace(scoped_name.begin(), scoped_name.end(), '/', '!');
  scoped_name.append("_service_run");
  return scoped_name;
}

class ServiceProcessTerminateMonitor
    : public base::win::ObjectWatcher::Delegate {
 public:
  explicit ServiceProcessTerminateMonitor(const base::Closure& terminate_task)
      : terminate_task_(terminate_task) {
  }
  void Start() {
    base::string16 event_name = GetServiceProcessTerminateEventName();
    DCHECK(event_name.length() <= MAX_PATH);
    terminate_event_.Set(CreateEvent(NULL, TRUE, FALSE, event_name.c_str()));
    watcher_.StartWatchingOnce(terminate_event_.Get(), this);
  }

  // base::ObjectWatcher::Delegate implementation.
  void OnObjectSignaled(HANDLE object) override {
    if (!terminate_task_.is_null()) {
      terminate_task_.Run();
      terminate_task_.Reset();
    }
  }

 private:
  base::win::ScopedHandle terminate_event_;
  base::win::ObjectWatcher watcher_;
  base::Closure terminate_task_;
};

}  // namespace

// Gets the name of the service process IPC channel.
mojo::edk::NamedPlatformHandle GetServiceProcessChannel() {
  return mojo::edk::NamedPlatformHandle(
      GetServiceProcessScopedVersionedName("_service_ipc"));
}

bool ForceServiceProcessShutdown(const std::string& version,
                                 base::ProcessId process_id) {
  base::win::ScopedHandle terminate_event;
  std::string versioned_name = version;
  versioned_name.append(kTerminateEventSuffix);
  base::string16 event_name =
      base::UTF8ToWide(GetServiceProcessScopedName(versioned_name));
  terminate_event.Set(OpenEvent(EVENT_MODIFY_STATE, FALSE, event_name.c_str()));
  if (!terminate_event.IsValid())
    return false;
  SetEvent(terminate_event.Get());
  return true;
}

bool CheckServiceProcessReady() {
  base::string16 event_name = GetServiceProcessReadyEventName();
  base::win::ScopedHandle event(
      OpenEvent(SYNCHRONIZE | READ_CONTROL, false, event_name.c_str()));
  if (!event.IsValid())
    return false;
  // Check if the event is signaled.
  return WaitForSingleObject(event.Get(), 0) == WAIT_OBJECT_0;
}

struct ServiceProcessState::StateData {
  // An event that is signaled when a service process is ready.
  base::win::ScopedHandle ready_event;
  std::unique_ptr<ServiceProcessTerminateMonitor> terminate_monitor;
};

void ServiceProcessState::CreateState() {
  DCHECK(!state_);
  state_ = new StateData;
}

bool ServiceProcessState::TakeSingletonLock() {
  DCHECK(state_);
  base::string16 event_name = GetServiceProcessReadyEventName();
  DCHECK(event_name.length() <= MAX_PATH);
  base::win::ScopedHandle service_process_ready_event;
  service_process_ready_event.Set(
      CreateEvent(NULL, TRUE, FALSE, event_name.c_str()));
  DWORD error = GetLastError();
  if ((error == ERROR_ALREADY_EXISTS) || (error == ERROR_ACCESS_DENIED))
    return false;
  DCHECK(service_process_ready_event.IsValid());
  state_->ready_event.Set(service_process_ready_event.Take());
  return true;
}

bool ServiceProcessState::SignalReady(
    scoped_refptr<base::SingleThreadTaskRunner> task_runner,
    const base::Closure& terminate_task) {
  DCHECK(state_);
  DCHECK(state_->ready_event.IsValid());
  if (!SetEvent(state_->ready_event.Get())) {
    return false;
  }
  if (!terminate_task.is_null()) {
    state_->terminate_monitor.reset(
        new ServiceProcessTerminateMonitor(terminate_task));
    state_->terminate_monitor->Start();
  }
  return true;
}

bool ServiceProcessState::AddToAutoRun() {
  DCHECK(autorun_command_line_.get());
  // Remove the old autorun value first because we changed the naming scheme
  // for the autorun value name.
  base::win::RemoveCommandFromAutoRun(
      HKEY_CURRENT_USER,
      base::UTF8ToWide(GetObsoleteServiceProcessAutoRunKey()));
  return base::win::AddCommandToAutoRun(
      HKEY_CURRENT_USER,
      base::UTF8ToWide(GetServiceProcessAutoRunKey()),
      autorun_command_line_->GetCommandLineString());
}

bool ServiceProcessState::RemoveFromAutoRun() {
  // Remove the old autorun value first because we changed the naming scheme
  // for the autorun value name.
  base::win::RemoveCommandFromAutoRun(
      HKEY_CURRENT_USER,
      base::UTF8ToWide(GetObsoleteServiceProcessAutoRunKey()));
  return base::win::RemoveCommandFromAutoRun(
      HKEY_CURRENT_USER, base::UTF8ToWide(GetServiceProcessAutoRunKey()));
}

void ServiceProcessState::TearDownState() {
  delete state_;
  state_ = NULL;
}

// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/task_manager/task_manager_interface.h"

#include "chrome/browser/browser_process.h"
#include "chrome/browser/task_manager/sampling/task_manager_impl.h"
#include "chrome/browser/task_manager/sampling/task_manager_io_thread_helper.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/pref_names.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/resource_request_info.h"

#if defined(OS_MACOSX)
#include "chrome/browser/ui/browser_dialogs.h"
#endif  // defined(OS_MACOSX)

namespace task_manager {

namespace {
BytesTransferredKey KeyForRequest(const net::URLRequest& request) {
  // Only net::URLRequestJob instances created by the ResourceDispatcherHost
  // have an associated ResourceRequestInfo and a render frame associated.
  // All other jobs will have -1 returned for the render process child and
  // routing ids - the jobs may still match a resource based on their origin id,
  // otherwise BytesRead() will attribute the activity to the Browser resource.
  const content::ResourceRequestInfo* info =
      content::ResourceRequestInfo::ForRequest(&request);
  int child_id = -1;
  int route_id = -1;

  if (info)
    info->GetAssociatedRenderFrame(&child_id, &route_id);

  // Get the origin PID of the request's originator.  This will only be set for
  // plugins - for renderer or browser initiated requests it will be zero.
  int origin_pid = info ? info->GetOriginPID() : 0;
  return {origin_pid, child_id, route_id};
}
}  // namespace

// static
void TaskManagerInterface::RegisterPrefs(PrefRegistrySimple* registry) {
  registry->RegisterDictionaryPref(prefs::kTaskManagerWindowPlacement);
  registry->RegisterDictionaryPref(prefs::kTaskManagerColumnVisibility);
  registry->RegisterBooleanPref(prefs::kTaskManagerEndProcessEnabled, true);
}

// static
bool TaskManagerInterface::IsEndProcessEnabled() {
  PrefService* state = g_browser_process->local_state();
  return !state || state->GetBoolean(prefs::kTaskManagerEndProcessEnabled);
}

// static
TaskManagerInterface* TaskManagerInterface::GetTaskManager() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  return TaskManagerImpl::GetInstance();
}

// static
void TaskManagerInterface::OnRawBytesRead(const net::URLRequest& request,
                                          int64_t bytes_read) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
  BytesTransferredKey key = KeyForRequest(request);
  TaskManagerIoThreadHelper::OnRawBytesTransferred(key, bytes_read,
                                                   0 /*bytes_sent*/);
}

// static
void TaskManagerInterface::OnRawBytesSent(const net::URLRequest& request,
                                          int64_t bytes_sent) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
  BytesTransferredKey key = KeyForRequest(request);
  TaskManagerIoThreadHelper::OnRawBytesTransferred(key, 0 /*bytes_read*/,
                                                   bytes_sent);
}

void TaskManagerInterface::AddObserver(TaskManagerObserver* observer) {
  observers_.AddObserver(observer);
  observer->observed_task_manager_ = this;

  ResourceFlagsAdded(observer->desired_resources_flags());

  base::TimeDelta current_refresh_time = GetCurrentRefreshTime();
  if (current_refresh_time == base::TimeDelta::Max()) {
    // This is the first observer to be added. Start updating.
    StartUpdating();
  }

  if (observer->desired_refresh_time() > current_refresh_time)
    return;

  // Reached here, then this is EITHER (not the first observer to be added AND
  // it requires a more frequent refresh rate) OR (it's the very first observer
  // to be added).
  // Reset the refresh timer.
  ScheduleRefresh(observer->desired_refresh_time());
}

void TaskManagerInterface::RemoveObserver(TaskManagerObserver* observer) {
  observers_.RemoveObserver(observer);
  observer->observed_task_manager_ = nullptr;

  // Recalculate the minimum refresh rate and the enabled resource flags.
  int64_t flags = 0;
  base::TimeDelta min_time = base::TimeDelta::Max();
  for (auto& observer : observers_) {
    if (observer.desired_refresh_time() < min_time)
      min_time = observer.desired_refresh_time();

    flags |= observer.desired_resources_flags();
  }

  if (min_time == base::TimeDelta::Max()) {
    // This is the last observer to be removed. Stop updating.
    SetEnabledResourceFlags(0);
    refresh_timer_->Stop();
    StopUpdating();
  } else {
    SetEnabledResourceFlags(flags);
    ScheduleRefresh(min_time);
  }
}

void TaskManagerInterface::RecalculateRefreshFlags() {
  int64_t flags = 0;
  for (auto& observer : observers_)
    flags |= observer.desired_resources_flags();

  SetEnabledResourceFlags(flags);
}

bool TaskManagerInterface::IsResourceRefreshEnabled(RefreshType type) const {
  return (enabled_resources_flags_ & type) != 0;
}

TaskManagerInterface::TaskManagerInterface()
    : refresh_timer_(new base::Timer(true, true)),
      enabled_resources_flags_(0) {
}

TaskManagerInterface::~TaskManagerInterface() {
}

void TaskManagerInterface::NotifyObserversOnTaskAdded(TaskId id) {
  for (TaskManagerObserver& observer : observers_)
    observer.OnTaskAdded(id);
}

void TaskManagerInterface::NotifyObserversOnTaskToBeRemoved(TaskId id) {
  for (TaskManagerObserver& observer : observers_)
    observer.OnTaskToBeRemoved(id);
}

void TaskManagerInterface::NotifyObserversOnRefresh(
    const TaskIdList& task_ids) {
  for (TaskManagerObserver& observer : observers_)
    observer.OnTasksRefreshed(task_ids);
}

void TaskManagerInterface::NotifyObserversOnRefreshWithBackgroundCalculations(
      const TaskIdList& task_ids) {
  for (TaskManagerObserver& observer : observers_)
    observer.OnTasksRefreshedWithBackgroundCalculations(task_ids);
}

void TaskManagerInterface::NotifyObserversOnTaskUnresponsive(TaskId id) {
  for (TaskManagerObserver& observer : observers_)
    observer.OnTaskUnresponsive(id);
}

base::TimeDelta TaskManagerInterface::GetCurrentRefreshTime() const {
  return refresh_timer_->IsRunning() ? refresh_timer_->GetCurrentDelay()
                                     : base::TimeDelta::Max();
}

void TaskManagerInterface::ResourceFlagsAdded(int64_t flags) {
  enabled_resources_flags_ |= flags;
}

void TaskManagerInterface::SetEnabledResourceFlags(int64_t flags) {
  enabled_resources_flags_ = flags;
}

void TaskManagerInterface::ScheduleRefresh(base::TimeDelta refresh_time) {
  refresh_timer_->Start(FROM_HERE,
                        refresh_time,
                        base::Bind(&TaskManagerInterface::Refresh,
                                   base::Unretained(this)));
}

}  // namespace task_manager

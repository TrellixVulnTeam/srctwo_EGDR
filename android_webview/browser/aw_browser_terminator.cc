// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "android_webview/browser/aw_browser_terminator.h"

#include <unistd.h>

#include "android_webview/browser/aw_render_process_gone_delegate.h"
#include "android_webview/common/aw_descriptors.h"
#include "android_webview/common/crash_reporter/aw_microdump_crash_reporter.h"
#include "base/bind.h"
#include "base/logging.h"
#include "base/stl_util.h"
#include "base/sync_socket.h"
#include "base/task_scheduler/post_task.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/child_process_data.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/render_widget_host_iterator.h"
#include "content/public/browser/web_contents.h"
#include "jni/AwBrowserProcess_jni.h"

using content::BrowserThread;

namespace android_webview {

namespace {

void GetAwRenderProcessGoneDelegatesForRenderProcess(
    int render_process_id,
    std::vector<AwRenderProcessGoneDelegate*>* delegates) {
  content::RenderProcessHost* rph =
      content::RenderProcessHost::FromID(render_process_id);
  if (!rph)
    return;

  std::unique_ptr<content::RenderWidgetHostIterator> widgets(
      content::RenderWidgetHost::GetRenderWidgetHosts());
  while (content::RenderWidgetHost* widget = widgets->GetNextHost()) {
    content::RenderViewHost* view = content::RenderViewHost::From(widget);
    if (view && rph == view->GetProcess()) {
      content::WebContents* wc = content::WebContents::FromRenderViewHost(view);
      if (wc) {
        AwRenderProcessGoneDelegate* delegate =
            AwRenderProcessGoneDelegate::FromWebContents(wc);
        if (delegate)
          delegates->push_back(delegate);
      }
    }
  }
}

void OnRenderProcessGone(int child_process_id) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  std::vector<AwRenderProcessGoneDelegate*> delegates;
  GetAwRenderProcessGoneDelegatesForRenderProcess(child_process_id, &delegates);
  for (auto* delegate : delegates)
    delegate->OnRenderProcessGone(child_process_id);
}

void OnRenderProcessGoneDetail(int child_process_id,
                               base::ProcessHandle child_process_pid,
                               bool crashed) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  std::vector<AwRenderProcessGoneDelegate*> delegates;
  GetAwRenderProcessGoneDelegatesForRenderProcess(child_process_id, &delegates);
  for (auto* delegate : delegates) {
    if (!delegate->OnRenderProcessGoneDetail(child_process_pid, crashed)) {
      if (crashed) {
        crash_reporter::SuppressDumpGeneration();
        // Keeps this log unchanged, CTS test uses it to detect crash.
        LOG(FATAL) << "Render process (" << child_process_pid << ")'s crash"
                   << " wasn't handled by all associated  webviews, triggering"
                   << " application crash.";
      } else {
        // The render process was most likely killed for OOM or switching
        // WebView provider, to make WebView backward compatible, kills the
        // browser process instead of triggering crash.
        LOG(ERROR) << "Render process (" << child_process_pid << ") kill (OOM"
                   << " or update) wasn't handed by all associated webviews,"
                   << " killing application.";
        kill(getpid(), SIGKILL);
      }
    }
  }

  // By this point we have moved the minidump to the crash directory, so it can
  // now be copied and uploaded.
  Java_AwBrowserProcess_triggerMinidumpUploading(
      base::android::AttachCurrentThread());
}

}  // namespace

AwBrowserTerminator::AwBrowserTerminator(base::FilePath crash_dump_dir)
    : crash_dump_dir_(crash_dump_dir) {}

AwBrowserTerminator::~AwBrowserTerminator() {}

void AwBrowserTerminator::OnChildStart(
    int child_process_id,
    content::PosixFileDescriptorInfo* mappings) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::PROCESS_LAUNCHER);

  base::AutoLock auto_lock(child_process_id_to_pipe_lock_);
  DCHECK(!ContainsKey(child_process_id_to_pipe_, child_process_id));

  auto local_pipe = base::MakeUnique<base::SyncSocket>();
  auto child_pipe = base::MakeUnique<base::SyncSocket>();
  if (base::SyncSocket::CreatePair(local_pipe.get(), child_pipe.get())) {
    child_process_id_to_pipe_[child_process_id] = std::move(local_pipe);
    mappings->Transfer(kAndroidWebViewCrashSignalDescriptor,
                       base::ScopedFD(dup(child_pipe->handle())));
  }
  if (crash_reporter::IsCrashReporterEnabled()) {
    base::ScopedFD file(
        breakpad::CrashDumpManager::GetInstance()->CreateMinidumpFileForChild(
            child_process_id));
    if (file != base::kInvalidPlatformFile)
      mappings->Transfer(kAndroidMinidumpDescriptor, std::move(file));
  }
}

void AwBrowserTerminator::OnChildExitAsync(
    int child_process_id,
    base::ProcessHandle pid,
    content::ProcessType process_type,
    base::TerminationStatus termination_status,
    base::android::ApplicationState app_state,
    base::FilePath crash_dump_dir,
    std::unique_ptr<base::SyncSocket> pipe) {
  if (crash_reporter::IsCrashReporterEnabled()) {
    breakpad::CrashDumpManager::GetInstance()->ProcessMinidumpFileFromChild(
        crash_dump_dir, child_process_id, process_type, termination_status,
        app_state);
  }

  if (!pipe.get() ||
      termination_status == base::TERMINATION_STATUS_NORMAL_TERMINATION)
    return;

  bool crashed = false;

  // If the child process hasn't written anything into the pipe. This implies
  // that it was terminated via SIGKILL by the low memory killer.
  if (pipe->Peek() >= sizeof(int)) {
    int exit_code;
    pipe->Receive(&exit_code, sizeof(exit_code));
    LOG(ERROR) << "Renderer process (" << pid << ") crash detected (code "
               << exit_code << ").";
    crashed = true;
  }

  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&OnRenderProcessGoneDetail, child_process_id, pid, crashed));
}

void AwBrowserTerminator::OnChildExit(
    int child_process_id,
    base::ProcessHandle pid,
    content::ProcessType process_type,
    base::TerminationStatus termination_status,
    base::android::ApplicationState app_state) {
  std::unique_ptr<base::SyncSocket> pipe;

  {
    base::AutoLock auto_lock(child_process_id_to_pipe_lock_);
    // We might get a NOTIFICATION_RENDERER_PROCESS_TERMINATED and a
    // NOTIFICATION_RENDERER_PROCESS_CLOSED. In that case we only want
    // to process the first notification.
    const auto& iter = child_process_id_to_pipe_.find(child_process_id);
    if (iter != child_process_id_to_pipe_.end()) {
      pipe = std::move(iter->second);
      DCHECK(pipe->handle() != base::SyncSocket::kInvalidHandle);
      child_process_id_to_pipe_.erase(iter);
    }
  }
  if (pipe.get()) {
    OnRenderProcessGone(child_process_id);
  }

  base::PostTaskWithTraits(
      FROM_HERE,
      {base::MayBlock(), base::TaskPriority::USER_VISIBLE,
       base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN},
      base::Bind(&AwBrowserTerminator::OnChildExitAsync, child_process_id, pid,
                 process_type, termination_status, app_state, crash_dump_dir_,
                 base::Passed(std::move(pipe))));
}

}  // namespace android_webview

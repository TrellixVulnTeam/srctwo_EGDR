// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/io_thread_extension_message_filter.h"

#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/app_window/app_window.h"
#include "extensions/browser/app_window/app_window_registry.h"
#include "extensions/browser/extension_function_dispatcher.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/info_map.h"
#include "extensions/common/extension_messages.h"
#include "ipc/ipc_message_macros.h"

using content::BrowserThread;

namespace extensions {

namespace {

void NotifyAppWindowReadyOnUIThread(int render_process_id, int route_id) {
  content::RenderProcessHost* process =
      content::RenderProcessHost::FromID(render_process_id);
  if (!process)
    return;
  content::BrowserContext* browser_context = process->GetBrowserContext();
  if (!browser_context)
    return;
  content::RenderViewHost* rvh =
      content::RenderViewHost::FromID(render_process_id, route_id);
  if (!rvh)
    return;
  content::WebContents* contents =
      content::WebContents::FromRenderViewHost(rvh);
  if (!contents)
    return;
  AppWindowRegistry* registry = AppWindowRegistry::Get(browser_context);
  DCHECK(registry);
  AppWindow* window = registry->GetAppWindowForWebContents(contents);
  if (window)
    window->NotifyRenderViewReady();
}

}  // namespace

IOThreadExtensionMessageFilter::IOThreadExtensionMessageFilter(
    int render_process_id,
    content::BrowserContext* context)
    : BrowserMessageFilter(ExtensionMsgStart),
      render_process_id_(render_process_id),
      browser_context_id_(context),
      extension_info_map_(ExtensionSystem::Get(context)->info_map()),
      weak_ptr_factory_(this) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
}

IOThreadExtensionMessageFilter::~IOThreadExtensionMessageFilter() {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
}

void IOThreadExtensionMessageFilter::OnDestruct() const {
  // Destroy the filter on the IO thread since that's where its weak pointers
  // are being used.
  BrowserThread::DeleteOnIOThread::Destruct(this);
}

bool IOThreadExtensionMessageFilter::OnMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(IOThreadExtensionMessageFilter, message)
    IPC_MESSAGE_HANDLER(ExtensionHostMsg_AppWindowReady, OnAppWindowReady);
  IPC_MESSAGE_HANDLER(ExtensionHostMsg_GenerateUniqueID,
                      OnExtensionGenerateUniqueID)
  IPC_MESSAGE_HANDLER(ExtensionHostMsg_RequestForIOThread,
                      OnExtensionRequestForIOThread)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void IOThreadExtensionMessageFilter::OnAppWindowReady(int route_id) {
  BrowserThread::PostTask(BrowserThread::UI, FROM_HERE,
                          base::Bind(&NotifyAppWindowReadyOnUIThread,
                                     render_process_id_, route_id));
}

void IOThreadExtensionMessageFilter::OnExtensionGenerateUniqueID(
    int* unique_id) {
  static int next_unique_id = 0;
  *unique_id = ++next_unique_id;
}

void IOThreadExtensionMessageFilter::OnExtensionRequestForIOThread(
    int routing_id,
    const ExtensionHostMsg_Request_Params& params) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  ExtensionFunctionDispatcher::DispatchOnIOThread(
      extension_info_map_.get(), browser_context_id_, render_process_id_,
      weak_ptr_factory_.GetWeakPtr(), routing_id, params);
}

}  // namespace extensions

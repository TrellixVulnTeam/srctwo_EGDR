// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/chrome_extension_host_delegate.h"

#include <memory>
#include <string>

#include "base/lazy_instance.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/data_use_measurement/data_use_web_contents_observer.h"
#include "chrome/browser/extensions/chrome_extension_web_contents_observer.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/extension_tab_util.h"
#include "chrome/browser/media/webrtc/media_capture_devices_dispatcher.h"
#include "chrome/browser/ui/prefs/prefs_tab_helper.h"
#include "components/app_modal/javascript_dialog_manager.h"
#include "extensions/browser/extension_host.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/load_monitoring_extension_host_queue.h"
#include "extensions/browser/serial_extension_host_queue.h"

namespace extensions {

namespace {

// Singleton for GetExtensionHostQueue().
struct QueueWrapper {
  QueueWrapper() {
    queue.reset(new LoadMonitoringExtensionHostQueue(
        std::unique_ptr<ExtensionHostQueue>(new SerialExtensionHostQueue())));
  }
  std::unique_ptr<ExtensionHostQueue> queue;
};
base::LazyInstance<QueueWrapper>::DestructorAtExit g_queue =
    LAZY_INSTANCE_INITIALIZER;

}  // namespace

ChromeExtensionHostDelegate::ChromeExtensionHostDelegate() {}

ChromeExtensionHostDelegate::~ChromeExtensionHostDelegate() {}

void ChromeExtensionHostDelegate::OnExtensionHostCreated(
    content::WebContents* web_contents) {
  ChromeExtensionWebContentsObserver::CreateForWebContents(web_contents);
  data_use_measurement::DataUseWebContentsObserver::CreateForWebContents(
      web_contents);
  PrefsTabHelper::CreateForWebContents(web_contents);
}

void ChromeExtensionHostDelegate::OnRenderViewCreatedForBackgroundPage(
    ExtensionHost* host) {
  ExtensionService* service =
      ExtensionSystem::Get(host->browser_context())->extension_service();
  if (service)
    service->DidCreateRenderViewForBackgroundPage(host);
}

content::JavaScriptDialogManager*
ChromeExtensionHostDelegate::GetJavaScriptDialogManager() {
  return app_modal::JavaScriptDialogManager::GetInstance();
}

void ChromeExtensionHostDelegate::CreateTab(content::WebContents* web_contents,
                                            const std::string& extension_id,
                                            WindowOpenDisposition disposition,
                                            const gfx::Rect& initial_rect,
                                            bool user_gesture) {
  // Verify that the browser is not shutting down. It can be the case if the
  // call is propagated through a posted task that was already in the queue when
  // shutdown started. See crbug.com/625646
  if (g_browser_process->IsShuttingDown())
    return;

  ExtensionTabUtil::CreateTab(
      web_contents, extension_id, disposition, initial_rect, user_gesture);
}

void ChromeExtensionHostDelegate::ProcessMediaAccessRequest(
    content::WebContents* web_contents,
    const content::MediaStreamRequest& request,
    const content::MediaResponseCallback& callback,
    const Extension* extension) {
  MediaCaptureDevicesDispatcher::GetInstance()->ProcessMediaAccessRequest(
      web_contents, request, callback, extension);
}

bool ChromeExtensionHostDelegate::CheckMediaAccessPermission(
    content::WebContents* web_contents,
    const GURL& security_origin,
    content::MediaStreamType type,
    const Extension* extension) {
  return MediaCaptureDevicesDispatcher::GetInstance()
      ->CheckMediaAccessPermission(
          web_contents, security_origin, type, extension);
}

ExtensionHostQueue* ChromeExtensionHostDelegate::GetExtensionHostQueue() const {
  return g_queue.Get().queue.get();
}

}  // namespace extensions

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/metrics/desktop_session_duration/desktop_session_duration_observer.h"

#include "base/memory/ptr_util.h"
#include "chrome/browser/metrics/desktop_session_duration/desktop_session_duration_tracker.h"
#include "content/public/browser/render_view_host.h"

DEFINE_WEB_CONTENTS_USER_DATA_KEY(metrics::DesktopSessionDurationObserver);

namespace metrics {

DesktopSessionDurationObserver::DesktopSessionDurationObserver(
    content::WebContents* web_contents,
    DesktopSessionDurationTracker* service)
    : content::WebContentsObserver(web_contents), service_(service) {
  RegisterInputEventObserver(web_contents->GetRenderViewHost());
}

DesktopSessionDurationObserver::~DesktopSessionDurationObserver() {}

// static
DesktopSessionDurationObserver*
DesktopSessionDurationObserver::CreateForWebContents(
    content::WebContents* web_contents) {
  DCHECK(web_contents);

  if (!DesktopSessionDurationTracker::IsInitialized())
    return nullptr;

  DesktopSessionDurationObserver* observer = FromWebContents(web_contents);
  if (!observer) {
    observer = new DesktopSessionDurationObserver(
        web_contents, DesktopSessionDurationTracker::Get());
    web_contents->SetUserData(UserDataKey(), base::WrapUnique(observer));
  }
  return observer;
}

void DesktopSessionDurationObserver::RegisterInputEventObserver(
    content::RenderViewHost* host) {
  if (host != nullptr)
    host->GetWidget()->AddInputEventObserver(this);
}

void DesktopSessionDurationObserver::UnregisterInputEventObserver(
    content::RenderViewHost* host) {
  if (host != nullptr)
    host->GetWidget()->RemoveInputEventObserver(this);
}

void DesktopSessionDurationObserver::OnInputEvent(
    const blink::WebInputEvent& event) {
  service_->OnUserEvent();
}

void DesktopSessionDurationObserver::RenderViewHostChanged(
    content::RenderViewHost* old_host,
    content::RenderViewHost* new_host) {
  UnregisterInputEventObserver(old_host);
  RegisterInputEventObserver(new_host);
}

}  // namespace metrics

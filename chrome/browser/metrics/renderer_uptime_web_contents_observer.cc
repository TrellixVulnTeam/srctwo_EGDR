// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/metrics/renderer_uptime_web_contents_observer.h"

#include "base/memory/ptr_util.h"
#include "chrome/browser/metrics/renderer_uptime_tracker.h"
#include "content/public/browser/render_process_host.h"

DEFINE_WEB_CONTENTS_USER_DATA_KEY(metrics::RendererUptimeWebContentsObserver);

namespace metrics {

RendererUptimeWebContentsObserver::RendererUptimeWebContentsObserver(
    content::WebContents* web_contents)
    : content::WebContentsObserver(web_contents) {}

// static
RendererUptimeWebContentsObserver*
RendererUptimeWebContentsObserver::CreateForWebContents(
    content::WebContents* web_contents) {
  DCHECK(web_contents);

  RendererUptimeWebContentsObserver* observer = FromWebContents(web_contents);
  if (!observer) {
    observer = new RendererUptimeWebContentsObserver(web_contents);
    web_contents->SetUserData(UserDataKey(), base::WrapUnique(observer));
  }
  return observer;
}

void RendererUptimeWebContentsObserver::DocumentAvailableInMainFrame() {
  RendererUptimeTracker::Get()->OnLoadInMainFrame(
      web_contents()->GetRenderProcessHost()->GetID());
}

}  // namespace metrics

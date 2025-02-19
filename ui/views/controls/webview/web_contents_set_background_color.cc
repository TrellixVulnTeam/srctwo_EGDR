// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/controls/webview/web_contents_set_background_color.h"

#include "base/memory/ptr_util.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/render_widget_host_view.h"

DEFINE_WEB_CONTENTS_USER_DATA_KEY(views::WebContentsSetBackgroundColor);

namespace views {

// static
void WebContentsSetBackgroundColor::CreateForWebContentsWithColor(
    content::WebContents* web_contents,
    SkColor color) {
  if (FromWebContents(web_contents))
    return;

  // SupportsUserData::Data takes ownership over the
  // WebContentsSetBackgroundColor instance and will destroy it when the
  // WebContents instance is destroyed.
  web_contents->SetUserData(
      UserDataKey(),
      base::WrapUnique(new WebContentsSetBackgroundColor(web_contents, color)));
}

WebContentsSetBackgroundColor::WebContentsSetBackgroundColor(
    content::WebContents* web_contents,
    SkColor color)
    : content::WebContentsObserver(web_contents), color_(color) {}

WebContentsSetBackgroundColor::~WebContentsSetBackgroundColor() {}

void WebContentsSetBackgroundColor::RenderViewReady() {
  web_contents()
      ->GetRenderViewHost()
      ->GetWidget()
      ->GetView()
      ->SetBackgroundColor(color_);
}

void WebContentsSetBackgroundColor::RenderViewCreated(
    content::RenderViewHost* render_view_host) {
  render_view_host->GetWidget()->GetView()->SetBackgroundColor(color_);
}

void WebContentsSetBackgroundColor::RenderViewHostChanged(
    content::RenderViewHost* old_host,
    content::RenderViewHost* new_host) {
  new_host->GetWidget()->GetView()->SetBackgroundColor(color_);
}

}  // namespace views

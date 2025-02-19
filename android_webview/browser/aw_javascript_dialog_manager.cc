// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "android_webview/browser/aw_javascript_dialog_manager.h"

#include "android_webview/browser/aw_contents_client_bridge.h"
#include "content/public/browser/javascript_dialog_manager.h"
#include "content/public/browser/web_contents.h"

namespace android_webview {

AwJavaScriptDialogManager::AwJavaScriptDialogManager() {}

AwJavaScriptDialogManager::~AwJavaScriptDialogManager() {}

void AwJavaScriptDialogManager::RunJavaScriptDialog(
    content::WebContents* web_contents,
    const GURL& origin_url,
    content::JavaScriptDialogType dialog_type,
    const base::string16& message_text,
    const base::string16& default_prompt_text,
    const DialogClosedCallback& callback,
    bool* did_suppress_message) {
  AwContentsClientBridge* bridge =
      AwContentsClientBridge::FromWebContents(web_contents);
  if (!bridge) {
    callback.Run(false, base::string16());
    return;
  }

  bridge->RunJavaScriptDialog(dialog_type, origin_url, message_text,
                              default_prompt_text, callback);
}

void AwJavaScriptDialogManager::RunBeforeUnloadDialog(
    content::WebContents* web_contents,
    bool is_reload,
    const DialogClosedCallback& callback) {
  AwContentsClientBridge* bridge =
      AwContentsClientBridge::FromWebContents(web_contents);
  if (!bridge) {
    callback.Run(false, base::string16());
    return;
  }

  bridge->RunBeforeUnloadDialog(web_contents->GetURL(),
                                callback);
}

void AwJavaScriptDialogManager::CancelDialogs(
    content::WebContents* web_contents,
    bool reset_state) {}

}  // namespace android_webview

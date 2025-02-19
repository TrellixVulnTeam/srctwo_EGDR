// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ANDROID_WEBVIEW_BROWSER_AW_JAVASCRIPT_DIALOG_MANAGER_H_
#define ANDROID_WEBVIEW_BROWSER_AW_JAVASCRIPT_DIALOG_MANAGER_H_

#include "base/macros.h"
#include "content/public/browser/javascript_dialog_manager.h"

namespace android_webview {

class AwJavaScriptDialogManager : public content::JavaScriptDialogManager {
 public:
  AwJavaScriptDialogManager();
  ~AwJavaScriptDialogManager() override;

  // Overridden from content::JavaScriptDialogManager:
  void RunJavaScriptDialog(content::WebContents* web_contents,
                           const GURL& origin_url,
                           content::JavaScriptDialogType dialog_type,
                           const base::string16& message_text,
                           const base::string16& default_prompt_text,
                           const DialogClosedCallback& callback,
                           bool* did_suppress_message) override;
  void RunBeforeUnloadDialog(content::WebContents* web_contents,
                             bool is_reload,
                             const DialogClosedCallback& callback) override;
  void CancelDialogs(content::WebContents* web_contents,
                     bool reset_state) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(AwJavaScriptDialogManager);
};

}  // namespace android_webview

#endif  // ANDROID_WEBVIEW_BROWSER_AW_JAVASCRIPT_DIALOG_MANAGER_H_

// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_SHELL_BROWSER_SHELL_APP_VIEW_GUEST_DELEGATE_H_
#define EXTENSIONS_SHELL_BROWSER_SHELL_APP_VIEW_GUEST_DELEGATE_H_

#include "base/macros.h"
#include "content/public/common/context_menu_params.h"
#include "extensions/browser/guest_view/app_view/app_view_guest_delegate.h"

namespace extensions {

class ShellAppViewGuestDelegate : public AppViewGuestDelegate {
 public:
  ShellAppViewGuestDelegate();
  ~ShellAppViewGuestDelegate() override;

  // AppViewGuestDelegate:
  bool HandleContextMenu(content::WebContents* web_contents,
                         const content::ContextMenuParams& params) override;
  AppDelegate* CreateAppDelegate() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(ShellAppViewGuestDelegate);
};

}  // namespace extensions

#endif  // EXTENSIONS_SHELL_BROWSER_SHELL_APP_VIEW_GUEST_DELEGATE_H_

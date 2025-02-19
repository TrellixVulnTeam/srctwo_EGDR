// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/toolbar/media_router_action_platform_delegate_views.h"

#include "base/memory/ptr_util.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "chrome/browser/ui/views/toolbar/app_menu_button.h"
#include "chrome/browser/ui/views/toolbar/toolbar_view.h"

// static
std::unique_ptr<MediaRouterActionPlatformDelegate>
MediaRouterActionPlatformDelegate::Create(Browser* browser) {
  return base::WrapUnique(new MediaRouterActionPlatformDelegateViews(browser));
}

MediaRouterActionPlatformDelegateViews::MediaRouterActionPlatformDelegateViews(
    Browser* browser)
    : MediaRouterActionPlatformDelegate(),
      browser_(browser) {
  DCHECK(browser_);
}

MediaRouterActionPlatformDelegateViews::
    ~MediaRouterActionPlatformDelegateViews() {
}

bool MediaRouterActionPlatformDelegateViews::CloseOverflowMenuIfOpen() {
  AppMenuButton* app_menu_button =
      BrowserView::GetBrowserViewForBrowser(browser_)
          ->toolbar()
          ->app_menu_button();
  if (!app_menu_button->IsMenuShowing())
    return false;

  app_menu_button->CloseMenu();
  return true;
}

// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/apps/chrome_native_app_window_views_win.h"

#include "apps/ui/views/app_window_frame_view.h"
#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/sequenced_worker_pool.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/shell_integration_win.h"
#include "chrome/browser/ui/views/apps/app_window_desktop_native_widget_aura_win.h"
#include "chrome/browser/ui/views/apps/glass_app_window_frame_view_win.h"
#include "chrome/browser/web_applications/web_app.h"
#include "chrome/browser/web_applications/web_app_win.h"
#include "chrome/common/chrome_switches.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/browser/app_window/app_window.h"
#include "extensions/browser/app_window/app_window_registry.h"
#include "extensions/common/extension.h"
#include "ui/base/win/shell.h"
#include "ui/views/widget/desktop_aura/desktop_native_widget_aura.h"
#include "ui/views/win/hwnd_util.h"

ChromeNativeAppWindowViewsWin::ChromeNativeAppWindowViewsWin()
    : glass_frame_view_(NULL), is_translucent_(false), weak_ptr_factory_(this) {
}

ChromeNativeAppWindowViewsWin::~ChromeNativeAppWindowViewsWin() {
}

HWND ChromeNativeAppWindowViewsWin::GetNativeAppWindowHWND() const {
  return views::HWNDForWidget(widget()->GetTopLevelWidget());
}

void ChromeNativeAppWindowViewsWin::EnsureCaptionStyleSet() {
  // Windows seems to have issues maximizing windows without WS_CAPTION.
  // The default views / Aura implementation will remove this if we are using
  // frameless or colored windows, so we put it back here.
  HWND hwnd = GetNativeAppWindowHWND();
  int current_style = ::GetWindowLong(hwnd, GWL_STYLE);
  ::SetWindowLong(hwnd, GWL_STYLE, current_style | WS_CAPTION);
}

void ChromeNativeAppWindowViewsWin::OnBeforeWidgetInit(
    const extensions::AppWindow::CreateParams& create_params,
    views::Widget::InitParams* init_params,
    views::Widget* widget) {
  ChromeNativeAppWindowViewsAura::OnBeforeWidgetInit(create_params, init_params,
                                                     widget);
  init_params->native_widget = new AppWindowDesktopNativeWidgetAuraWin(this);

  is_translucent_ =
      init_params->opacity == views::Widget::InitParams::TRANSLUCENT_WINDOW;
}

void ChromeNativeAppWindowViewsWin::InitializeDefaultWindow(
    const extensions::AppWindow::CreateParams& create_params) {
  ChromeNativeAppWindowViewsAura::InitializeDefaultWindow(create_params);

  const extensions::Extension* extension = app_window()->GetExtension();
  if (!extension)
    return;

  std::string app_name =
      web_app::GenerateApplicationNameFromExtensionId(extension->id());
  base::string16 app_name_wide = base::UTF8ToWide(app_name);
  HWND hwnd = GetNativeAppWindowHWND();
  Profile* profile =
      Profile::FromBrowserContext(app_window()->browser_context());
  app_model_id_ = shell_integration::win::GetAppModelIdForProfile(
      app_name_wide, profile->GetPath());
  ui::win::SetAppIdForWindow(app_model_id_, hwnd);
  web_app::UpdateRelaunchDetailsForApp(profile, extension, hwnd);

  if (!create_params.alpha_enabled)
    EnsureCaptionStyleSet();
}

views::NonClientFrameView*
ChromeNativeAppWindowViewsWin::CreateStandardDesktopAppFrame() {
  glass_frame_view_ = NULL;
  if (ui::win::IsAeroGlassEnabled()) {
    glass_frame_view_ = new GlassAppWindowFrameViewWin(widget());
    return glass_frame_view_;
  }
  return ChromeNativeAppWindowViewsAura::CreateStandardDesktopAppFrame();
}

bool ChromeNativeAppWindowViewsWin::CanMinimize() const {
  // Resizing on Windows breaks translucency if the window also has shape.
  // See http://crbug.com/417947.
  return ChromeNativeAppWindowViewsAura::CanMinimize() &&
         !(WidgetHasHitTestMask() && is_translucent_);
}

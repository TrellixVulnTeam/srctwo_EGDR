// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_FRAME_DESKTOP_BROWSER_FRAME_AURAX11_H_
#define CHROME_BROWSER_UI_VIEWS_FRAME_DESKTOP_BROWSER_FRAME_AURAX11_H_

#include "base/macros.h"
#include "chrome/browser/ui/views/frame/desktop_browser_frame_aura.h"
#include "components/prefs/pref_member.h"

// Provides the window frame for the Chrome browser window on Desktop Linux/X11.
class DesktopBrowserFrameAuraX11 : public DesktopBrowserFrameAura {
 public:
  DesktopBrowserFrameAuraX11(BrowserFrame* browser_frame,
                             BrowserView* browser_view);

 protected:
  ~DesktopBrowserFrameAuraX11() override;

  // Overridden from NativeBrowserFrame:
  views::Widget::InitParams GetWidgetParams() override;
  bool UseCustomFrame() const override;

 private:
  // Called when the preference changes.
  void OnUseCustomChromeFrameChanged();

  // Whether the custom Chrome frame preference is set.
  BooleanPrefMember use_custom_frame_pref_;

  DISALLOW_COPY_AND_ASSIGN(DesktopBrowserFrameAuraX11);
};

#endif  // CHROME_BROWSER_UI_VIEWS_FRAME_DESKTOP_BROWSER_FRAME_AURAX11_H_

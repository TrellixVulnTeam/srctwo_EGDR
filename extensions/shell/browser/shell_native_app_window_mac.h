// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_SHELL_BROWSER_SHELL_NATIVE_APP_WINDOW_MAC_H_
#define EXTENSIONS_SHELL_BROWSER_SHELL_NATIVE_APP_WINDOW_MAC_H_

#import <Cocoa/Cocoa.h>

#import "base/mac/scoped_nsobject.h"
#include "base/macros.h"
#include "extensions/shell/browser/shell_native_app_window.h"

@class ShellNSWindow;

namespace extensions {
class ShellNativeAppWindowMac;
}

// A window controller for ShellNativeAppWindowMac to handle NSNotifications
// and pass them to the C++ implementation.
@interface ShellNativeAppWindowController
    : NSWindowController<NSWindowDelegate> {
 @private
  extensions::ShellNativeAppWindowMac* appWindow_;  // Owns us.
}

@property(assign, nonatomic) extensions::ShellNativeAppWindowMac* appWindow;

@end

namespace extensions {

// A minimal implementation of ShellNativeAppWindow for Mac Cocoa.
// Based on the NativeAppWindowCocoa implementation.
class ShellNativeAppWindowMac : public ShellNativeAppWindow {
 public:
  ShellNativeAppWindowMac(extensions::AppWindow* app_window,
                          const extensions::AppWindow::CreateParams& params);
  ~ShellNativeAppWindowMac() override;

  // ui::BaseWindow:
  bool IsActive() const override;
  gfx::NativeWindow GetNativeWindow() const override;
  gfx::Rect GetBounds() const override;
  void Show() override;
  void Hide() override;
  void Activate() override;
  void Deactivate() override;
  void SetBounds(const gfx::Rect& bounds) override;

  // NativeAppWindow:
  gfx::Size GetContentMinimumSize() const override;
  gfx::Size GetContentMaximumSize() const override;

  // Called when the window is about to close.
  void WindowWillClose();

 private:
  ShellNSWindow* window() const;

  base::scoped_nsobject<ShellNativeAppWindowController> window_controller_;

  DISALLOW_COPY_AND_ASSIGN(ShellNativeAppWindowMac);
};

}  // namespace extensions

#endif  // EXTENSIONS_SHELL_BROWSER_SHELL_NATIVE_APP_WINDOW_MAC_H_

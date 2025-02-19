// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_SHELL_BROWSER_SHELL_NATIVE_APP_WINDOW_AURA_H_
#define EXTENSIONS_SHELL_BROWSER_SHELL_NATIVE_APP_WINDOW_AURA_H_

#include "base/macros.h"
#include "extensions/shell/browser/shell_native_app_window.h"

namespace extensions {

// The Aura-specific parts of the app_shell NativeAppWindow implementation.
class ShellNativeAppWindowAura : public ShellNativeAppWindow {
 public:
  ShellNativeAppWindowAura(extensions::AppWindow* app_window,
                           const AppWindow::CreateParams& params);
  ~ShellNativeAppWindowAura() override;

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

 private:
  DISALLOW_COPY_AND_ASSIGN(ShellNativeAppWindowAura);
};

}  // namespace extensions

#endif  // EXTENSIONS_SHELL_BROWSER_SHELL_NATIVE_APP_WINDOW_AURA_H_

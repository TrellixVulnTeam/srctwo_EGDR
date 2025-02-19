// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_INSTALL_EXTENSION_INSTALL_UI_H_
#define EXTENSIONS_BROWSER_INSTALL_EXTENSION_INSTALL_UI_H_

#include <string>

#include "base/macros.h"
#include "ui/gfx/native_widget_types.h"

class SkBitmap;

namespace extensions {
class CrxInstallError;
class Extension;

// Interface that should be implemented for each platform to display all the UI
// around extension installation.
class ExtensionInstallUI {
 public:
  ExtensionInstallUI();
  virtual ~ExtensionInstallUI();

  // Called when an extension was installed.
  virtual void OnInstallSuccess(const extensions::Extension* extension,
                                const SkBitmap* icon) = 0;

  // Called when an extension failed to install.
  virtual void OnInstallFailure(const extensions::CrxInstallError& error) = 0;

  // TODO(asargent) Normally we navigate to the new tab page when an app is
  // installed, but we're experimenting with instead showing a bubble when
  // an app is installed which points to the new tab button. This may become
  // the default behavior in the future.
  virtual void SetUseAppInstalledBubble(bool use_bubble) = 0;

  // Opens apps UI and animates the app icon for the app with id |app_id|.
  virtual void OpenAppInstalledUI(const std::string& app_id) = 0;

  // Sets whether to show the default UI after completing the installation.
  virtual void SetSkipPostInstallUI(bool skip_ui) = 0;

  // Returns the gfx::NativeWindow to use as the parent for install dialogs.
  // Returns NULL if the install dialog should be a top level window. This
  // method is deprecated - do not add new callers.
  // TODO(pkotwicz): Remove this method. crbug.com/422474
  virtual gfx::NativeWindow GetDefaultInstallDialogParent() = 0;

#if defined(UNIT_TEST)
  static void set_disable_failure_ui_for_tests() {
    disable_failure_ui_for_tests_ = true;
  }
#endif

 protected:
  static bool disable_failure_ui_for_tests() {
    return disable_failure_ui_for_tests_;
  }

 private:
  static bool disable_failure_ui_for_tests_;

  DISALLOW_COPY_AND_ASSIGN(ExtensionInstallUI);
};

}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_INSTALL_EXTENSION_INSTALL_UI_H_

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_DEVICE_KEYBOARD_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_DEVICE_KEYBOARD_HANDLER_H_

#include "base/macros.h"
#include "base/scoped_observer.h"
#include "chrome/browser/ui/webui/settings/settings_page_ui_handler.h"
#include "ui/events/devices/input_device_event_observer.h"

namespace base {
class ListValue;
}

namespace ui {
class InputDeviceManager;
}

namespace chromeos {
namespace settings {

// Chrome OS "Keyboard" settings page UI handler.
class KeyboardHandler
    : public ::settings::SettingsPageUIHandler,
      public ui::InputDeviceEventObserver {
 public:
  // Name of the message sent to WebUI when the keys that should be shown
  // change.
  static const char kShowKeysChangedName[];

  // Class used by tests to interact with KeyboardHandler internals.
  class TestAPI {
   public:
    explicit TestAPI(KeyboardHandler* handler) { handler_ = handler; }

    // Simulates a request from WebUI to initialize the page.
    void Initialize();

   private:
    KeyboardHandler* handler_;  // Not owned.
    DISALLOW_COPY_AND_ASSIGN(TestAPI);
  };

  KeyboardHandler();
  ~KeyboardHandler() override;

  // SettingsPageUIHandler implementation.
  void RegisterMessages() override;
  void OnJavascriptAllowed() override;
  void OnJavascriptDisallowed() override;

  // ui::InputDeviceEventObserver implementation.
  void OnKeyboardDeviceConfigurationChanged() override;

 private:
  // Initializes the page with the current keyboard information.
  void HandleInitialize(const base::ListValue* args);

  // Shows the Ash keyboard shortcuts overlay.
  void HandleShowKeyboardShortcutsOverlay(const base::ListValue* args) const;

  // Shows or hides the Caps Lock and Diamond key settings based on whether the
  // system status.
  void UpdateShowKeys();

  ScopedObserver<ui::InputDeviceManager, KeyboardHandler> observer_;

  DISALLOW_COPY_AND_ASSIGN(KeyboardHandler);
};

}  // namespace settings
}  // namespace chromeos

#endif  // CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_DEVICE_KEYBOARD_HANDLER_H_

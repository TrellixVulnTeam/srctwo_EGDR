// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_FOCUS_WIDGET_FOCUS_MANAGER_H_
#define UI_VIEWS_FOCUS_WIDGET_FOCUS_MANAGER_H_

#include "base/macros.h"
#include "base/observer_list.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/views/views_export.h"

namespace base {
template <typename T> struct DefaultSingletonTraits;
}

namespace views {

// This interface should be implemented by classes that want to be notified when
// the native focus is about to change.  Listeners implementing this interface
// will be invoked for all native focus changes across the entire Chrome
// application.  FocusChangeListeners are only called for changes within the
// children of a single top-level native-view.
class WidgetFocusChangeListener {
 public:
  virtual void OnNativeFocusChanged(gfx::NativeView focused_now) = 0;

 protected:
  virtual ~WidgetFocusChangeListener() {}
};

class VIEWS_EXPORT WidgetFocusManager {
 public:
  // Returns the singleton instance.
  static WidgetFocusManager* GetInstance();

  // Adds/removes a WidgetFocusChangeListener |listener| to the set of
  // active listeners.
  void AddFocusChangeListener(WidgetFocusChangeListener* listener);
  void RemoveFocusChangeListener(WidgetFocusChangeListener* listener);

  // Called when native-focus shifts within, or off, a widget. |focused_now| is
  // null when focus is lost.
  void OnNativeFocusChanged(gfx::NativeView focused_now);

  // Enable/Disable notification of registered listeners during calls
  // to OnNativeFocusChanged.  Used to prevent unwanted focus changes from
  // propagating notifications.
  void EnableNotifications() { enabled_ = true; }
  void DisableNotifications() { enabled_ = false; }

 private:
  friend struct base::DefaultSingletonTraits<WidgetFocusManager>;

  WidgetFocusManager();
  ~WidgetFocusManager();

  base::ObserverList<WidgetFocusChangeListener> focus_change_listeners_;

  bool enabled_;

  DISALLOW_COPY_AND_ASSIGN(WidgetFocusManager);
};

// A basic helper class that is used to disable native focus change
// notifications within a scope.
class VIEWS_EXPORT AutoNativeNotificationDisabler {
 public:
  AutoNativeNotificationDisabler();
  ~AutoNativeNotificationDisabler();

 private:
  DISALLOW_COPY_AND_ASSIGN(AutoNativeNotificationDisabler);
};

}  // namespace views

#endif  // UI_VIEWS_FOCUS_WIDGET_FOCUS_MANAGER_H_

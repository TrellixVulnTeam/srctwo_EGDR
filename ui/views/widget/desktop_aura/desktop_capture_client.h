// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_CAPTURE_CLIENT_H_
#define UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_CAPTURE_CLIENT_H_

#include <set>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/observer_list.h"
#include "ui/aura/client/capture_client.h"
#include "ui/views/views_export.h"

namespace aura {
class RootWindow;
}

namespace views {

// Desktop implementation of CaptureClient. There is one CaptureClient per
// DesktopNativeWidgetAura.
//
// DesktopCaptureClient and CaptureController (used by ash) differ slightly in
// how they handle capture. CaptureController is a singleton shared among all
// RootWindows created by ash. An implication of this is that all RootWindows
// know which window has capture. This is not the case with
// DesktopCaptureClient. Instead each RootWindow has its own
// DesktopCaptureClient. This means only the RootWindow of the Window that has
// capture knows which window has capture. All others think no one has
// capture. This behavior is necessitated by Windows occassionally delivering
// mouse events to a window other than the capture window and expecting that
// window to get the event. If we shared the capture window on the desktop this
// behavior would not be possible.
class VIEWS_EXPORT DesktopCaptureClient : public aura::client::CaptureClient {
 public:
  explicit DesktopCaptureClient(aura::Window* root);
  ~DesktopCaptureClient() override;

  // Exactly the same as GetGlobalCaptureWindow() but static.
  static aura::Window* GetCaptureWindowGlobal();

  // Overridden from aura::client::CaptureClient:
  void SetCapture(aura::Window* window) override;
  void ReleaseCapture(aura::Window* window) override;
  aura::Window* GetCaptureWindow() override;
  aura::Window* GetGlobalCaptureWindow() override;
  void AddObserver(aura::client::CaptureClientObserver* observer) override;
  void RemoveObserver(aura::client::CaptureClientObserver* observer) override;

 private:
  typedef std::set<DesktopCaptureClient*> CaptureClients;

  aura::Window* root_;
  aura::Window* capture_window_;

  // Set of DesktopCaptureClients.
  static CaptureClients* capture_clients_;

  base::ObserverList<aura::client::CaptureClientObserver> observers_;

  DISALLOW_COPY_AND_ASSIGN(DesktopCaptureClient);
};

}  // namespace views

#endif  // UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_CAPTURE_CLIENT_H_

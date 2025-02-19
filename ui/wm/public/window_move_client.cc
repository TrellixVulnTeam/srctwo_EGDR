// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/wm/public/window_move_client.h"

#include "ui/aura/window.h"
#include "ui/base/class_property.h"

DECLARE_UI_CLASS_PROPERTY_TYPE(wm::WindowMoveClient*)

namespace wm {

// A property key to store a client that handles window moves.
DEFINE_LOCAL_UI_CLASS_PROPERTY_KEY(
    WindowMoveClient*, kWindowMoveClientKey, NULL);

void SetWindowMoveClient(aura::Window* window, WindowMoveClient* client) {
  window->SetProperty(kWindowMoveClientKey, client);
}

WindowMoveClient* GetWindowMoveClient(aura::Window* window) {
  return window->GetProperty(kWindowMoveClientKey);
}

}  // namespace wm

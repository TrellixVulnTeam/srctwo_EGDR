// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/aura/test/test_window_parenting_client.h"

#include "ui/aura/window.h"

namespace aura {
namespace test {

TestWindowParentingClient::TestWindowParentingClient(Window* root_window)
    : root_window_(root_window) {
  client::SetWindowParentingClient(root_window_, this);
}

TestWindowParentingClient::~TestWindowParentingClient() {
  client::SetWindowParentingClient(root_window_, nullptr);
}

Window* TestWindowParentingClient::GetDefaultParent(Window* window,
                                                    const gfx::Rect& bounds) {
  return root_window_;
}

}  // namespace test
}  // namespace aura

// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/aura/scoped_window_targeter.h"

#include <utility>

#include "ui/aura/window.h"

namespace aura {

ScopedWindowTargeter::ScopedWindowTargeter(
    Window* window,
    std::unique_ptr<ui::EventTargeter> new_targeter)
    : window_(window),
      old_targeter_(window->SetEventTargeter(std::move(new_targeter))) {
  window_->AddObserver(this);
}

ScopedWindowTargeter::~ScopedWindowTargeter() {
  if (window_) {
    window_->RemoveObserver(this);
    window_->SetEventTargeter(std::move(old_targeter_));
  }
}

void ScopedWindowTargeter::OnWindowDestroyed(Window* window) {
  CHECK_EQ(window_, window);
  window_ = NULL;
  old_targeter_.reset();
}

}  // namespace aura

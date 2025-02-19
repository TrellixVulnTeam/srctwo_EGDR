// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/base/android/mock_android_overlay.h"

#include "base/bind.h"
#include "base/memory/ptr_util.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace media {

MockAndroidOverlay::Callbacks::Callbacks() = default;
MockAndroidOverlay::Callbacks::Callbacks(const Callbacks&) = default;
MockAndroidOverlay::Callbacks::~Callbacks() = default;

MockAndroidOverlay::MockAndroidOverlay() : weak_factory_(this) {}

MockAndroidOverlay::~MockAndroidOverlay() {}

void MockAndroidOverlay::SetConfig(AndroidOverlayConfig config) {
  config_ = base::MakeUnique<AndroidOverlayConfig>(std::move(config));
}

MockAndroidOverlay::Callbacks MockAndroidOverlay::GetCallbacks() {
  Callbacks c;
  c.OverlayReady = base::Bind(&MockAndroidOverlay::OnOverlayReady,
                              weak_factory_.GetWeakPtr());
  c.OverlayFailed = base::Bind(&MockAndroidOverlay::OnOverlayFailed,
                               weak_factory_.GetWeakPtr());
  c.SurfaceDestroyed = base::Bind(&MockAndroidOverlay::OnSurfaceDestroyed,
                                  weak_factory_.GetWeakPtr());

  return c;
}

void MockAndroidOverlay::OnOverlayReady() {
  config_->is_ready(this);
}

void MockAndroidOverlay::OnOverlayFailed() {
  config_->is_failed(this);
}

void MockAndroidOverlay::OnSurfaceDestroyed() {
  RunSurfaceDestroyedCallbacks();
}

}  // namespace media

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/arc/notification/arc_notification_surface_manager.h"

#include "base/logging.h"

namespace arc {

// static
ArcNotificationSurfaceManager* ArcNotificationSurfaceManager::instance_ =
    nullptr;

ArcNotificationSurfaceManager::ArcNotificationSurfaceManager() {
  DCHECK(!instance_);
  instance_ = this;
}

ArcNotificationSurfaceManager::~ArcNotificationSurfaceManager() {
  DCHECK_EQ(this, instance_);
  instance_ = nullptr;
}

// static
ArcNotificationSurfaceManager* ArcNotificationSurfaceManager::Get() {
  return instance_;
}

}  // namespace arc

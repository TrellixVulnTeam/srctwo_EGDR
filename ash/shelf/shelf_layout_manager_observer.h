// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_SHELF_SHELF_LAYOUT_MANAGER_OBSERVER_H_
#define ASH_SHELF_SHELF_LAYOUT_MANAGER_OBSERVER_H_

#include "ash/ash_export.h"
#include "ash/public/cpp/shelf_types.h"
#include "ash/shelf/shelf_background_animator.h"

namespace ash {

enum class AnimationChangeType;

class ASH_EXPORT ShelfLayoutManagerObserver {
 public:
  virtual ~ShelfLayoutManagerObserver() {}

  // Called when the target ShelfLayoutManager will be deleted.
  virtual void WillDeleteShelfLayoutManager() {}

  // Called when the visibility change is scheduled.
  virtual void WillChangeVisibilityState(ShelfVisibilityState new_state) {}

  // Called when the auto hide state is changed.
  virtual void OnAutoHideStateChanged(ShelfAutoHideState new_state) {}

  // Called when shelf background animation is started.
  virtual void OnBackgroundUpdated(ShelfBackgroundType background_type,
                                   AnimationChangeType change_type) {}
};

}  // namespace ash

#endif  // ASH_SHELF_SHELF_LAYOUT_MANAGER_OBSERVER_H_

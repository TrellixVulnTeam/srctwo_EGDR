// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_RENDERER_HOST_OVERSCROLL_CONTROLLER_DELEGATE_H_
#define CONTENT_BROWSER_RENDERER_HOST_OVERSCROLL_CONTROLLER_DELEGATE_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/optional.h"
#include "content/browser/renderer_host/overscroll_controller.h"
#include "content/common/content_export.h"
#include "ui/gfx/geometry/size.h"

namespace content {

// The delegate receives overscroll gesture updates from the controller and
// should perform appropriate actions. The delegate can optionally cap the
// overscroll deltas maintained and reported by the controller.
class CONTENT_EXPORT OverscrollControllerDelegate {
 public:
  OverscrollControllerDelegate() {}
  virtual ~OverscrollControllerDelegate() {}

  // Get the size of the display containing the view corresponding to the
  // delegate.
  virtual gfx::Size GetDisplaySize() const = 0;

  // This is called for each update in the overscroll amount. Returns true if
  // the delegate consumed the event.
  virtual bool OnOverscrollUpdate(float delta_x, float delta_y) = 0;

  // This is called when the overscroll completes.
  virtual void OnOverscrollComplete(OverscrollMode overscroll_mode) = 0;

  // This is called when the direction of the overscroll changes. When a new
  // overscroll is started (i.e. when |new_mode| is not equal to
  // OVERSCROLL_NONE), |source| will be set to the device that triggered the
  // overscroll gesture.
  virtual void OnOverscrollModeChange(OverscrollMode old_mode,
                                      OverscrollMode new_mode,
                                      OverscrollSource source) = 0;

  // Returns the optional maximum amount allowed for the absolute value of
  // overscroll delta corresponding to the current overscroll mode.
  virtual base::Optional<float> GetMaxOverscrollDelta() const = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(OverscrollControllerDelegate);
};

}  // namespace content

#endif  // CONTENT_BROWSER_RENDERER_HOST_OVERSCROLL_CONTROLLER_DELEGATE_H_

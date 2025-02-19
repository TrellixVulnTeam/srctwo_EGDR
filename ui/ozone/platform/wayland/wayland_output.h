// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_OZONE_PLATFORM_WAYLAND_WAYLAND_OUTPUT_H_
#define UI_OZONE_PLATFORM_WAYLAND_WAYLAND_OUTPUT_H_

#include <stdint.h>

#include "ui/gfx/geometry/rect.h"
#include "ui/ozone/platform/wayland/wayland_object.h"

namespace ui {

// WaylandOutput objects keep track of the current output of display
// that are available to the application.
class WaylandOutput {
 public:
  class Observer {
   public:
    // Will be called when wl_output is available.
    virtual void OnOutputReadyForUse() = 0;
  };

  WaylandOutput(wl_output*);
  ~WaylandOutput();

  // Returns the geometry of the output.
  gfx::Rect Geometry() const { return rect_; }
  void SetObserver(Observer* observer) { observer_ = observer; }
  Observer* observer() { return observer_; }

 private:
  // Callback functions used for setting geometric properties of the output
  // and available modes.
  static void OutputHandleGeometry(void* data,
                                   wl_output* output,
                                   int32_t x,
                                   int32_t y,
                                   int32_t physical_width,
                                   int32_t physical_height,
                                   int32_t subpixel,
                                   const char* make,
                                   const char* model,
                                   int32_t output_transform);

  static void OutputHandleMode(void* data,
                               wl_output* wl_output,
                               uint32_t flags,
                               int32_t width,
                               int32_t height,
                               int32_t refresh);

  wl::Object<wl_output> output_;
  gfx::Rect rect_;

  Observer* observer_;

  DISALLOW_COPY_AND_ASSIGN(WaylandOutput);
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_WAYLAND_SCREEN_H_

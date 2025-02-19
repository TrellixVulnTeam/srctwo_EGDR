// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_MUS_DRAG_WINDOW_RESIZER_H_
#define ASH_MUS_DRAG_WINDOW_RESIZER_H_

#include <memory>

#include "ash/wm/window_resizer.h"
#include "base/macros.h"

namespace ash {
namespace mus {

// DragWindowResizer is a decorator of WindowResizer and adds the ability to
// drag windows across displays.
class DragWindowResizer : public WindowResizer {
 public:
  DragWindowResizer(std::unique_ptr<WindowResizer> next_window_resizer,
                    wm::WindowState* window_state);
  ~DragWindowResizer() override;

  // WindowResizer:
  void Drag(const gfx::Point& location, int event_flags) override;
  void CompleteDrag() override;
  void RevertDrag() override;

 private:
  std::unique_ptr<WindowResizer> next_window_resizer_;

  DISALLOW_COPY_AND_ASSIGN(DragWindowResizer);
};

}  // namespace mus
}  // namespace ash

#endif  // ASH_MUS_DRAG_WINDOW_RESIZER_H_

// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_DISPLAY_ROOT_WINDOW_TRANSFORMERS_H_
#define ASH_DISPLAY_ROOT_WINDOW_TRANSFORMERS_H_

#include "ash/ash_export.h"

namespace aura {
class Window;
}

namespace display {
class Display;
class ManagedDisplayInfo;
}

namespace gfx {
class Rect;
}

namespace ash {
class RootWindowTransformer;

ASH_EXPORT RootWindowTransformer* CreateRootWindowTransformerForDisplay(
    aura::Window* root,
    const display::Display& display);

// Creates a RootWindowTransformers for mirror root window.
// |source_display_info| specifies the display being mirrored,
// and |mirror_display_info| specifies the display used to
// mirror the content.
ASH_EXPORT RootWindowTransformer* CreateRootWindowTransformerForMirroredDisplay(
    const display::ManagedDisplayInfo& source_display_info,
    const display::ManagedDisplayInfo& mirror_display_info);

// Creates a RootWindowTransformers for unified desktop mode.
// |screen_bounds| specifies the unified desktop's bounds and
// |display| specifies the display used to mirror the unified desktop.
ASH_EXPORT RootWindowTransformer* CreateRootWindowTransformerForUnifiedDesktop(
    const gfx::Rect& screen_bounds,
    const display::Display& display);

}  // namespace ash

#endif  // ASH_DISPLAY_ROOT_WINDOW_TRANSFORMERS_H_

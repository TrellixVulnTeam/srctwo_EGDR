// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/controls/menu/menu_image_util.h"

#include "ui/base/material_design/material_design_controller.h"
#include "ui/gfx/color_palette.h"
#include "ui/gfx/paint_vector_icon.h"
#include "ui/gfx/vector_icon_types.h"
#include "ui/views/vector_icons.h"

namespace views {

gfx::ImageSkia GetMenuCheckImage(SkColor icon_color) {
  return gfx::CreateVectorIcon(kMenuCheckIcon, icon_color);
}

gfx::ImageSkia GetRadioButtonImage(bool toggled,
                                   bool hovered,
                                   SkColor default_icon_color) {
  const gfx::VectorIcon& icon =
      toggled ? kMenuRadioSelectedIcon : kMenuRadioEmptyIcon;
  SkColor color =
      toggled && !hovered ? gfx::kGoogleBlue500 : default_icon_color;
  return gfx::CreateVectorIcon(icon, kMenuCheckSize, color);
}

gfx::ImageSkia GetSubmenuArrowImage(SkColor icon_color) {
  return gfx::CreateVectorIcon(kSubmenuArrowIcon, icon_color);
}

}  // namespace views

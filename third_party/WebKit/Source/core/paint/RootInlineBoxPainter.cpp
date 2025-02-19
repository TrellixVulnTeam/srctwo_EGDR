// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/paint/RootInlineBoxPainter.h"

#include "core/layout/api/LineLayoutAPIShim.h"
#include "core/layout/line/EllipsisBox.h"
#include "core/layout/line/RootInlineBox.h"
#include "core/paint/PaintInfo.h"

namespace blink {

void RootInlineBoxPainter::PaintEllipsisBox(const PaintInfo& paint_info,
                                            const LayoutPoint& paint_offset,
                                            LayoutUnit line_top,
                                            LayoutUnit line_bottom) const {
  if (root_inline_box_.HasEllipsisBox() &&
      root_inline_box_.GetLineLayoutItem().Style()->Visibility() ==
          EVisibility::kVisible &&
      paint_info.phase == kPaintPhaseForeground)
    root_inline_box_.GetEllipsisBox()->Paint(paint_info, paint_offset, line_top,
                                             line_bottom);
}

void RootInlineBoxPainter::Paint(const PaintInfo& paint_info,
                                 const LayoutPoint& paint_offset,
                                 LayoutUnit line_top,
                                 LayoutUnit line_bottom) {
  root_inline_box_.InlineFlowBox::Paint(paint_info, paint_offset, line_top,
                                        line_bottom);
  PaintEllipsisBox(paint_info, paint_offset, line_top, line_bottom);
}

}  // namespace blink

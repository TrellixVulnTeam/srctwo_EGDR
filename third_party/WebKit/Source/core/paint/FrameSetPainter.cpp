// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/paint/FrameSetPainter.h"

#include "core/html/HTMLFrameSetElement.h"
#include "core/layout/LayoutFrameSet.h"
#include "core/paint/LayoutObjectDrawingRecorder.h"
#include "core/paint/ObjectPainter.h"
#include "core/paint/PaintInfo.h"

namespace blink {

static Color BorderStartEdgeColor() {
  return Color(170, 170, 170);
}

static Color BorderEndEdgeColor() {
  return Color::kBlack;
}

static Color BorderFillColor() {
  return Color(208, 208, 208);
}

void FrameSetPainter::PaintColumnBorder(const PaintInfo& paint_info,
                                        const IntRect& border_rect) {
  if (!paint_info.GetCullRect().IntersectsCullRect(border_rect))
    return;

  // FIXME: We should do something clever when borders from distinct framesets
  // meet at a join.

  // Fill first.
  GraphicsContext& context = paint_info.context;
  context.FillRect(border_rect, layout_frame_set_.FrameSet()->HasBorderColor()
                                    ? layout_frame_set_.ResolveColor(
                                          CSSPropertyBorderLeftColor)
                                    : BorderFillColor());

  // Now stroke the edges but only if we have enough room to paint both edges
  // with a little bit of the fill color showing through.
  if (border_rect.Width() >= 3) {
    context.FillRect(
        IntRect(border_rect.Location(), IntSize(1, border_rect.Height())),
        BorderStartEdgeColor());
    context.FillRect(IntRect(IntPoint(border_rect.MaxX() - 1, border_rect.Y()),
                             IntSize(1, border_rect.Height())),
                     BorderEndEdgeColor());
  }
}

void FrameSetPainter::PaintRowBorder(const PaintInfo& paint_info,
                                     const IntRect& border_rect) {
  // FIXME: We should do something clever when borders from distinct framesets
  // meet at a join.

  // Fill first.
  GraphicsContext& context = paint_info.context;
  context.FillRect(border_rect, layout_frame_set_.FrameSet()->HasBorderColor()
                                    ? layout_frame_set_.ResolveColor(
                                          CSSPropertyBorderLeftColor)
                                    : BorderFillColor());

  // Now stroke the edges but only if we have enough room to paint both edges
  // with a little bit of the fill color showing through.
  if (border_rect.Height() >= 3) {
    context.FillRect(
        IntRect(border_rect.Location(), IntSize(border_rect.Width(), 1)),
        BorderStartEdgeColor());
    context.FillRect(IntRect(IntPoint(border_rect.X(), border_rect.MaxY() - 1),
                             IntSize(border_rect.Width(), 1)),
                     BorderEndEdgeColor());
  }
}

static bool ShouldPaintBorderAfter(const LayoutFrameSet::GridAxis& axis,
                                   size_t index) {
  // Should not paint a border after the last frame along the axis.
  return index + 1 < axis.sizes_.size() && axis.allow_border_[index + 1];
}

void FrameSetPainter::PaintBorders(const PaintInfo& paint_info,
                                   const LayoutPoint& adjusted_paint_offset) {
  if (LayoutObjectDrawingRecorder::UseCachedDrawingIfPossible(
          paint_info.context, layout_frame_set_, paint_info.phase))
    return;

  LayoutRect adjusted_frame_rect(adjusted_paint_offset,
                                 layout_frame_set_.Size());
  LayoutObjectDrawingRecorder recorder(paint_info.context, layout_frame_set_,
                                       paint_info.phase, adjusted_frame_rect);

  LayoutUnit border_thickness(layout_frame_set_.FrameSet()->Border());
  if (!border_thickness)
    return;

  LayoutObject* child = layout_frame_set_.FirstChild();
  size_t rows = layout_frame_set_.Rows().sizes_.size();
  size_t cols = layout_frame_set_.Columns().sizes_.size();
  LayoutUnit y_pos;
  for (size_t r = 0; r < rows; r++) {
    LayoutUnit x_pos;
    for (size_t c = 0; c < cols; c++) {
      x_pos += layout_frame_set_.Columns().sizes_[c];
      if (ShouldPaintBorderAfter(layout_frame_set_.Columns(), c)) {
        PaintColumnBorder(
            paint_info, PixelSnappedIntRect(LayoutRect(
                            adjusted_paint_offset.X() + x_pos,
                            adjusted_paint_offset.Y() + y_pos, border_thickness,
                            layout_frame_set_.Size().Height() - y_pos)));
        x_pos += border_thickness;
      }
      child = child->NextSibling();
      if (!child)
        return;
    }
    y_pos += layout_frame_set_.Rows().sizes_[r];
    if (ShouldPaintBorderAfter(layout_frame_set_.Rows(), r)) {
      PaintRowBorder(
          paint_info,
          PixelSnappedIntRect(LayoutRect(
              adjusted_paint_offset.X(), adjusted_paint_offset.Y() + y_pos,
              layout_frame_set_.Size().Width(), border_thickness)));
      y_pos += border_thickness;
    }
  }
}

void FrameSetPainter::PaintChildren(const PaintInfo& paint_info,
                                    const LayoutPoint& adjusted_paint_offset) {
  // Paint only those children that fit in the grid.
  // Remaining frames are "hidden".
  // See also LayoutFrameSet::positionFrames.
  LayoutObject* child = layout_frame_set_.FirstChild();
  size_t rows = layout_frame_set_.Rows().sizes_.size();
  size_t cols = layout_frame_set_.Columns().sizes_.size();
  for (size_t r = 0; r < rows; r++) {
    for (size_t c = 0; c < cols; c++) {
      // Self-painting layers are painted during the PaintLayer paint recursion,
      // not LayoutObject.
      if (!child->IsBoxModelObject() ||
          !ToLayoutBoxModelObject(child)->HasSelfPaintingLayer())
        child->Paint(paint_info, adjusted_paint_offset);
      child = child->NextSibling();
      if (!child)
        return;
    }
  }
}

void FrameSetPainter::Paint(const PaintInfo& paint_info,
                            const LayoutPoint& paint_offset) {
  ObjectPainter(layout_frame_set_).CheckPaintOffset(paint_info, paint_offset);
  if (paint_info.phase != kPaintPhaseForeground)
    return;

  LayoutObject* child = layout_frame_set_.FirstChild();
  if (!child)
    return;

  LayoutPoint adjusted_paint_offset =
      paint_offset + layout_frame_set_.Location();
  PaintChildren(paint_info, adjusted_paint_offset);
  PaintBorders(paint_info, adjusted_paint_offset);
}

}  // namespace blink

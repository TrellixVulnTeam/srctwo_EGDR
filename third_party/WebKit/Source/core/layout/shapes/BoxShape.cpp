/*
 * Copyright (C) 2013 Adobe Systems Incorporated. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "core/layout/shapes/BoxShape.h"

#include "platform/wtf/MathExtras.h"

namespace blink {

LayoutRect BoxShape::ShapeMarginLogicalBoundingBox() const {
  FloatRect margin_bounds(bounds_.Rect());
  if (ShapeMargin() > 0)
    margin_bounds.Inflate(ShapeMargin());
  return static_cast<LayoutRect>(margin_bounds);
}

FloatRoundedRect BoxShape::ShapeMarginBounds() const {
  FloatRoundedRect margin_bounds(bounds_);
  if (ShapeMargin() > 0) {
    margin_bounds.Inflate(ShapeMargin());
    margin_bounds.ExpandRadii(ShapeMargin());
  }
  return margin_bounds;
}

LineSegment BoxShape::GetExcludedInterval(LayoutUnit logical_top,
                                          LayoutUnit logical_height) const {
  const FloatRoundedRect& margin_bounds = ShapeMarginBounds();
  if (margin_bounds.IsEmpty() ||
      !LineOverlapsShapeMarginBounds(logical_top, logical_height))
    return LineSegment();

  float y1 = logical_top.ToFloat();
  float y2 = (logical_top + logical_height).ToFloat();
  const FloatRect& rect = margin_bounds.Rect();

  if (!margin_bounds.IsRounded())
    return LineSegment(margin_bounds.Rect().X(), margin_bounds.Rect().MaxX());

  float top_corner_max_y =
      std::max<float>(margin_bounds.TopLeftCorner().MaxY(),
                      margin_bounds.TopRightCorner().MaxY());
  float bottom_corner_min_y =
      std::min<float>(margin_bounds.BottomLeftCorner().Y(),
                      margin_bounds.BottomRightCorner().Y());

  if (top_corner_max_y <= bottom_corner_min_y && y1 <= top_corner_max_y &&
      y2 >= bottom_corner_min_y)
    return LineSegment(rect.X(), rect.MaxX());

  float x1 = rect.MaxX();
  float x2 = rect.X();
  float min_x_intercept;
  float max_x_intercept;

  if (y1 <= margin_bounds.TopLeftCorner().MaxY() &&
      y2 >= margin_bounds.BottomLeftCorner().Y())
    x1 = rect.X();

  if (y1 <= margin_bounds.TopRightCorner().MaxY() &&
      y2 >= margin_bounds.BottomRightCorner().Y())
    x2 = rect.MaxX();

  if (margin_bounds.XInterceptsAtY(y1, min_x_intercept, max_x_intercept)) {
    x1 = std::min<float>(x1, min_x_intercept);
    x2 = std::max<float>(x2, max_x_intercept);
  }

  if (margin_bounds.XInterceptsAtY(y2, min_x_intercept, max_x_intercept)) {
    x1 = std::min<float>(x1, min_x_intercept);
    x2 = std::max<float>(x2, max_x_intercept);
  }

  DCHECK_GE(x2, x1);
  return LineSegment(x1, x2);
}

void BoxShape::BuildDisplayPaths(DisplayPaths& paths) const {
  paths.shape.AddRoundedRect(bounds_.Rect(), bounds_.GetRadii().TopLeft(),
                             bounds_.GetRadii().TopRight(),
                             bounds_.GetRadii().BottomLeft(),
                             bounds_.GetRadii().BottomRight());
  if (ShapeMargin())
    paths.margin_shape.AddRoundedRect(
        ShapeMarginBounds().Rect(), ShapeMarginBounds().GetRadii().TopLeft(),
        ShapeMarginBounds().GetRadii().TopRight(),
        ShapeMarginBounds().GetRadii().BottomLeft(),
        ShapeMarginBounds().GetRadii().BottomRight());
}

}  // namespace blink

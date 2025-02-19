// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef AppliedDecorationPainter_h
#define AppliedDecorationPainter_h

#include "core/paint/DecorationInfo.h"
#include "core/style/AppliedTextDecoration.h"
#include "core/style/ComputedStyleConstants.h"
#include "platform/geometry/FloatPoint.h"
#include "platform/geometry/FloatRect.h"
#include "platform/geometry/LayoutRect.h"
#include "platform/graphics/Path.h"
#include "platform/wtf/Allocator.h"

namespace blink {

class GraphicsContext;

// Helper class for painting a text decorations. Each instance paints a single
// decoration.
class AppliedDecorationPainter final {
  STACK_ALLOCATED();

 public:
  AppliedDecorationPainter(GraphicsContext& context,
                           const DecorationInfo& decoration_info,
                           float start_point_y_offset,
                           const AppliedTextDecoration& decoration,
                           float double_offset,
                           int wavy_offset_factor)
      : context_(context),
        start_point_(decoration_info.local_origin +
                     FloatPoint(0, start_point_y_offset)),
        decoration_info_(decoration_info),
        decoration_(decoration),
        double_offset_(double_offset),
        wavy_offset_factor_(wavy_offset_factor){};

  void Paint();
  FloatRect Bounds();

 private:
  void StrokeWavyTextDecoration();

  Path PrepareWavyStrokePath();
  Path PrepareDottedDashedStrokePath();

  GraphicsContext& context_;
  const FloatPoint start_point_;
  const DecorationInfo& decoration_info_;
  const AppliedTextDecoration& decoration_;
  const float double_offset_;
  const int wavy_offset_factor_;
};

}  // namespace blink

#endif  // AppliedDecorationPainter_h

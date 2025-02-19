// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GraphicsContextCanvas_h
#define GraphicsContextCanvas_h

#include <ApplicationServices/ApplicationServices.h>

#include "platform/PlatformExport.h"
#include "platform/graphics/paint/PaintCanvas.h"
#include "third_party/skia/include/core/SkBitmap.h"

struct SkIRect;

namespace blink {

// Creates a bridge for painting into a PaintCanvas with a CGContext.
class PLATFORM_EXPORT GraphicsContextCanvas {
 public:
  // Internally creates a bitmap the same size |paint_rect|, scaled by
  // |bitmap_scale_factor|.  Painting into the CgContext will go into the
  // bitmap.  Upon destruction, that bitmap will be painted into the
  // canvas as the rectangle |paint_rect|.  Users are expected to
  // clip |paint_rect| to reasonable sizes to not create giant bitmaps.
  // The |paint_rect| is in canvas device space.  The CgContext is set
  // up to be in exactly the same space as the canvas is at construction
  // time.
  GraphicsContextCanvas(PaintCanvas*,
                        const SkIRect& paint_rect,
                        SkScalar bitmap_scale_factor = 1);
  ~GraphicsContextCanvas();

  CGContextRef CgContext();

 private:
  void ReleaseIfNeeded();

  PaintCanvas* canvas_;

  CGContextRef cg_context_;
  SkBitmap offscreen_;
  SkScalar bitmap_scale_factor_;

  SkIRect paint_rect_;
};

}  // namespace blink

#endif  // GraphicsContextCanvas_h

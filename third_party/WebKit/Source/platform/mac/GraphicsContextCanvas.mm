// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/mac/GraphicsContextCanvas.h"

#import <AppKit/AppKit.h>
#import <CoreGraphics/CoreGraphics.h>

#include "platform/RuntimeEnabledFeatures.h"
#include "platform/wtf/RetainPtr.h"
#include "skia/ext/skia_utils_mac.h"

namespace blink {

GraphicsContextCanvas::GraphicsContextCanvas(PaintCanvas* canvas,
                                             const SkIRect& paint_rect,
                                             SkScalar bitmap_scale_factor)
    : canvas_(canvas),
      cg_context_(0),
      bitmap_scale_factor_(bitmap_scale_factor),
      paint_rect_(paint_rect) {
  // Callers should just avoid painting at all when this is the case.
  DCHECK(!paint_rect_.isEmpty());
}

GraphicsContextCanvas::~GraphicsContextCanvas() {
  ReleaseIfNeeded();
}

// This must be called to balance calls to cgContext
void GraphicsContextCanvas::ReleaseIfNeeded() {
  if (!cg_context_)
    return;
  offscreen_.setImmutable();  // Prevents a defensive copy inside Skia.
  canvas_->save();
  canvas_->setMatrix(SkMatrix::I());  // Reset back to device space.
  canvas_->translate(paint_rect_.x(), paint_rect_.y());
  canvas_->scale(1.f / bitmap_scale_factor_, 1.f / bitmap_scale_factor_);
  canvas_->drawBitmap(offscreen_, 0, 0);
  canvas_->restore();

  CGContextRelease(cg_context_);
  cg_context_ = 0;
}

CGContextRef GraphicsContextCanvas::CgContext() {
  ReleaseIfNeeded();  // This flushes any prior bitmap use

  // Allocate an offscreen and draw into that, relying on the
  // compositing step to apply skia's clip.
  WTF::RetainPtr<CGColorSpace> color_space(
      RuntimeEnabledFeatures::ColorCorrectRenderingEnabled()
          ? CGColorSpaceCreateWithName(kCGColorSpaceSRGB)
          : CGColorSpaceCreateDeviceRGB());

  bool result = offscreen_.tryAllocN32Pixels(
      SkScalarCeilToInt(bitmap_scale_factor_ * paint_rect_.width()),
      SkScalarCeilToInt(bitmap_scale_factor_ * paint_rect_.height()));
  DCHECK(result);
  if (!result)
    return 0;
  offscreen_.eraseColor(0);
  int display_height = offscreen_.height();
  cg_context_ = CGBitmapContextCreate(
      offscreen_.getPixels(), offscreen_.width(), offscreen_.height(), 8,
      offscreen_.rowBytes(), color_space.Get(),
      kCGBitmapByteOrder32Host | kCGImageAlphaPremultipliedFirst);
  DCHECK(cg_context_);

  SkMatrix matrix = canvas_->getTotalMatrix();
  matrix.postTranslate(-SkIntToScalar(paint_rect_.x()),
                       -SkIntToScalar(paint_rect_.y()));
  matrix.postScale(bitmap_scale_factor_, -bitmap_scale_factor_);
  matrix.postTranslate(0, SkIntToScalar(display_height));

  CGContextConcatCTM(cg_context_, skia::SkMatrixToCGAffineTransform(matrix));

  return cg_context_;
}

}  // namespace blink

// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKIA_EXT_ANALYSIS_CANVAS_H_
#define SKIA_EXT_ANALYSIS_CANVAS_H_

#include <stddef.h>
#include <stdint.h>

#include "base/compiler_specific.h"
#include "third_party/skia/include/core/SkPicture.h"
#include "third_party/skia/include/utils/SkNoDrawCanvas.h"

namespace skia {

// Does not render anything, but gathers statistics about a region
// (specified as a clip rectangle) of an SkPicture as the picture is
// played back through it.
// To use: play a picture into the canvas, and then check result.
class SK_API AnalysisCanvas final : public SkNoDrawCanvas,
                                    public SkPicture::AbortCallback {
 public:
  AnalysisCanvas(int width, int height);
  ~AnalysisCanvas() override;

  // Returns true when a SkColor can be used to represent result.
  bool GetColorIfSolid(SkColor* color) const;

  void SetForceNotSolid(bool flag);
  void SetForceNotTransparent(bool flag);

  // SkPicture::AbortCallback override.
  bool abort() override;

  // SkCanvas overrides.
  void onDrawPaint(const SkPaint& paint) override;
  void onDrawPoints(PointMode,
                  size_t count,
                  const SkPoint pts[],
                  const SkPaint&) override;
  void onDrawOval(const SkRect&, const SkPaint&) override;
  void onDrawArc(const SkRect&,
                 SkScalar startAngle,
                 SkScalar sweepAngle,
                 bool useCenter,
                 const SkPaint&) override;
  void onDrawRect(const SkRect&, const SkPaint&) override;
  void onDrawRRect(const SkRRect&, const SkPaint&) override;
  void onDrawPath(const SkPath& path, const SkPaint&) override;
  void onDrawBitmap(const SkBitmap&,
                    SkScalar left,
                    SkScalar top,
                    const SkPaint* paint = NULL) override;
  void onDrawBitmapRect(const SkBitmap&,
                        const SkRect* src,
                        const SkRect& dst,
                        const SkPaint* paint,
                        SrcRectConstraint) override;
  void onDrawBitmapNine(const SkBitmap& bitmap,
                        const SkIRect& center,
                        const SkRect& dst,
                        const SkPaint* paint = NULL) override;
  void onDrawImage(const SkImage*,
                    SkScalar left,
                    SkScalar top,
                    const SkPaint* paint = NULL) override;
  void onDrawImageRect(const SkImage*,
                       const SkRect* src,
                       const SkRect& dst,
                       const SkPaint* paint,
                       SrcRectConstraint) override;
  void onDrawVerticesObject(const SkVertices*,
                            SkBlendMode,
                            const SkPaint&) override;

 protected:
  void willSave() override;
  SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) override;
  void willRestore() override;

  void onClipRect(const SkRect& rect,
                  SkClipOp op,
                  ClipEdgeStyle edge_style) override;
  void onClipRRect(const SkRRect& rrect,
                   SkClipOp op,
                   ClipEdgeStyle edge_style) override;
  void onClipPath(const SkPath& path,
                  SkClipOp op,
                  ClipEdgeStyle edge_style) override;
  void onClipRegion(const SkRegion& deviceRgn, SkClipOp op) override;

  void onDrawText(const void* text,
                  size_t byteLength,
                  SkScalar x,
                  SkScalar y,
                  const SkPaint&) override;
  void onDrawPosText(const void* text,
                     size_t byteLength,
                     const SkPoint pos[],
                     const SkPaint&) override;
  void onDrawPosTextH(const void* text,
                      size_t byteLength,
                      const SkScalar xpos[],
                      SkScalar constY,
                      const SkPaint&) override;
  void onDrawTextOnPath(const void* text,
                        size_t byteLength,
                        const SkPath& path,
                        const SkMatrix* matrix,
                        const SkPaint&) override;
  void onDrawTextBlob(const SkTextBlob* blob,
                      SkScalar x,
                      SkScalar y,
                      const SkPaint& paint) override;
  void onDrawDRRect(const SkRRect& outer,
                    const SkRRect& inner,
                    const SkPaint&) override;

  void OnComplexClip();

 private:
  typedef SkNoDrawCanvas INHERITED;

  int saved_stack_size_;
  int force_not_solid_stack_level_;
  int force_not_transparent_stack_level_;

  bool is_forced_not_solid_;
  bool is_forced_not_transparent_;
  bool is_solid_color_;
  SkColor color_;
  bool is_transparent_;
  int draw_op_count_;
  int rejected_op_count_;
};

}  // namespace skia

#endif  // SKIA_EXT_ANALYSIS_CANVAS_H_


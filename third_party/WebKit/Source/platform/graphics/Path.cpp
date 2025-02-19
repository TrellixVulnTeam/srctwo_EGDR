/*
 * Copyright (C) 2003, 2006 Apple Computer, Inc.  All rights reserved.
 *                     2006 Rob Buis <buis@kde.org>
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2013 Google Inc. All rights reserved.
 * Copyright (C) 2013 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "platform/graphics/Path.h"

#include <math.h>
#include "platform/geometry/FloatPoint.h"
#include "platform/geometry/FloatRect.h"
#include "platform/graphics/GraphicsContext.h"
#include "platform/graphics/skia/SkiaUtils.h"
#include "platform/transforms/AffineTransform.h"
#include "platform/wtf/MathExtras.h"
#include "third_party/skia/include/pathops/SkPathOps.h"

namespace blink {

Path::Path() : path_() {}

Path::Path(const Path& other) {
  path_ = SkPath(other.path_);
}

Path::Path(const SkPath& other) {
  path_ = other;
}

Path::~Path() {}

Path& Path::operator=(const Path& other) {
  path_ = SkPath(other.path_);
  return *this;
}

Path& Path::operator=(const SkPath& other) {
  path_ = other;
  return *this;
}

bool Path::operator==(const Path& other) const {
  return path_ == other.path_;
}

bool Path::Contains(const FloatPoint& point) const {
  return path_.contains(WebCoreFloatToSkScalar(point.X()),
                        WebCoreFloatToSkScalar(point.Y()));
}

bool Path::Contains(const FloatPoint& point, WindRule rule) const {
  SkScalar x = WebCoreFloatToSkScalar(point.X());
  SkScalar y = WebCoreFloatToSkScalar(point.Y());
  SkPath::FillType fill_type = WebCoreWindRuleToSkFillType(rule);
  if (path_.getFillType() != fill_type) {
    SkPath tmp(path_);
    tmp.setFillType(fill_type);
    return tmp.contains(x, y);
  }
  return path_.contains(x, y);
}

// FIXME: this method ignores the CTM and may yield inaccurate results for large
// scales.
SkPath Path::StrokePath(const StrokeData& stroke_data) const {
  PaintFlags flags;
  stroke_data.SetupPaint(&flags);

  // Skia stroke resolution scale. This is multiplied by 4 internally
  // (i.e. 1.0 corresponds to 1/4 pixel res).
  static const SkScalar kResScale = 0.3f;

  SkPath stroke_path;
  flags.getFillPath(path_, &stroke_path, nullptr, kResScale);

  return stroke_path;
}

bool Path::StrokeContains(const FloatPoint& point,
                          const StrokeData& stroke_data) const {
  return StrokePath(stroke_data)
      .contains(WebCoreFloatToSkScalar(point.X()),
                WebCoreFloatToSkScalar(point.Y()));
}

namespace {

FloatRect PathBounds(const SkPath& path, Path::BoundsType bounds_type) {
  return bounds_type == Path::BoundsType::kConservative
             ? path.getBounds()
             : path.computeTightBounds();
}

}  // anonymous ns

// TODO(fmalita): evaluate returning exact bounds in all cases.
FloatRect Path::BoundingRect(BoundsType bounds_type) const {
  return PathBounds(path_, bounds_type);
}

FloatRect Path::StrokeBoundingRect(const StrokeData& stroke_data,
                                   BoundsType bounds_type) const {
  return PathBounds(StrokePath(stroke_data), bounds_type);
}

static FloatPoint* ConvertPathPoints(FloatPoint dst[],
                                     const SkPoint src[],
                                     int count) {
  for (int i = 0; i < count; i++) {
    dst[i].SetX(SkScalarToFloat(src[i].fX));
    dst[i].SetY(SkScalarToFloat(src[i].fY));
  }
  return dst;
}

void Path::Apply(void* info, PathApplierFunction function) const {
  SkPath::RawIter iter(path_);
  SkPoint pts[4];
  PathElement path_element;
  FloatPoint path_points[3];

  for (;;) {
    switch (iter.next(pts)) {
      case SkPath::kMove_Verb:
        path_element.type = kPathElementMoveToPoint;
        path_element.points = ConvertPathPoints(path_points, &pts[0], 1);
        break;
      case SkPath::kLine_Verb:
        path_element.type = kPathElementAddLineToPoint;
        path_element.points = ConvertPathPoints(path_points, &pts[1], 1);
        break;
      case SkPath::kQuad_Verb:
        path_element.type = kPathElementAddQuadCurveToPoint;
        path_element.points = ConvertPathPoints(path_points, &pts[1], 2);
        break;
      case SkPath::kCubic_Verb:
        path_element.type = kPathElementAddCurveToPoint;
        path_element.points = ConvertPathPoints(path_points, &pts[1], 3);
        break;
      case SkPath::kConic_Verb: {
        // Approximate with quads.  Use two for now, increase if more precision
        // is needed.
        const int kPow2 = 1;
        const unsigned kQuadCount = 1 << kPow2;
        SkPoint quads[1 + 2 * kQuadCount];
        SkPath::ConvertConicToQuads(pts[0], pts[1], pts[2], iter.conicWeight(),
                                    quads, kPow2);

        path_element.type = kPathElementAddQuadCurveToPoint;
        for (unsigned i = 0; i < kQuadCount; ++i) {
          path_element.points =
              ConvertPathPoints(path_points, &quads[1 + 2 * i], 2);
          function(info, &path_element);
        }
        continue;
      }
      case SkPath::kClose_Verb:
        path_element.type = kPathElementCloseSubpath;
        path_element.points = ConvertPathPoints(path_points, 0, 0);
        break;
      case SkPath::kDone_Verb:
        return;
    }
    function(info, &path_element);
  }
}

void Path::Transform(const AffineTransform& xform) {
  path_.transform(AffineTransformToSkMatrix(xform));
}

float Path::length() const {
  SkScalar length = 0;
  SkPathMeasure measure(path_, false);

  do {
    length += measure.getLength();
  } while (measure.nextContour());

  return SkScalarToFloat(length);
}

FloatPoint Path::PointAtLength(float length) const {
  FloatPoint point;
  float normal;
  PointAndNormalAtLength(length, point, normal);
  return point;
}

static bool CalculatePointAndNormalOnPath(SkPathMeasure& measure,
                                          SkScalar length,
                                          FloatPoint& point,
                                          float& normal_angle,
                                          SkScalar* accumulated_length = 0) {
  do {
    SkScalar contour_length = measure.getLength();
    if (length <= contour_length) {
      SkVector tangent;
      SkPoint position;

      if (measure.getPosTan(length, &position, &tangent)) {
        normal_angle =
            rad2deg(SkScalarToFloat(SkScalarATan2(tangent.fY, tangent.fX)));
        point = FloatPoint(SkScalarToFloat(position.fX),
                           SkScalarToFloat(position.fY));
        return true;
      }
    }
    length -= contour_length;
    if (accumulated_length)
      *accumulated_length += contour_length;
  } while (measure.nextContour());
  return false;
}

void Path::PointAndNormalAtLength(float length,
                                  FloatPoint& point,
                                  float& normal) const {
  SkPathMeasure measure(path_, false);
  if (CalculatePointAndNormalOnPath(measure, WebCoreFloatToSkScalar(length),
                                    point, normal))
    return;

  SkPoint position = path_.getPoint(0);
  point =
      FloatPoint(SkScalarToFloat(position.fX), SkScalarToFloat(position.fY));
  normal = 0;
}

Path::PositionCalculator::PositionCalculator(const Path& path)
    : path_(path.GetSkPath()),
      path_measure_(path.GetSkPath(), false),
      accumulated_length_(0) {}

void Path::PositionCalculator::PointAndNormalAtLength(float length,
                                                      FloatPoint& point,
                                                      float& normal_angle) {
  SkScalar sk_length = WebCoreFloatToSkScalar(length);
  if (sk_length >= 0) {
    if (sk_length < accumulated_length_) {
      // Reset path measurer to rewind (and restart from 0).
      path_measure_.setPath(&path_, false);
      accumulated_length_ = 0;
    } else {
      sk_length -= accumulated_length_;
    }

    if (CalculatePointAndNormalOnPath(path_measure_, sk_length, point,
                                      normal_angle, &accumulated_length_))
      return;
  }

  SkPoint position = path_.getPoint(0);
  point =
      FloatPoint(SkScalarToFloat(position.fX), SkScalarToFloat(position.fY));
  normal_angle = 0;
  return;
}

void Path::Clear() {
  path_.reset();
}

bool Path::IsEmpty() const {
  return path_.isEmpty();
}

bool Path::IsClosed() const {
  return path_.isLastContourClosed();
}

void Path::SetIsVolatile(bool is_volatile) {
  path_.setIsVolatile(is_volatile);
}

bool Path::HasCurrentPoint() const {
  return path_.getPoints(0, 0);
}

FloatPoint Path::CurrentPoint() const {
  if (path_.countPoints() > 0) {
    SkPoint sk_result;
    path_.getLastPt(&sk_result);
    FloatPoint result;
    result.SetX(SkScalarToFloat(sk_result.fX));
    result.SetY(SkScalarToFloat(sk_result.fY));
    return result;
  }

  // FIXME: Why does this return quietNaN? Other ports return 0,0.
  float quiet_na_n = std::numeric_limits<float>::quiet_NaN();
  return FloatPoint(quiet_na_n, quiet_na_n);
}

void Path::SetWindRule(const WindRule rule) {
  path_.setFillType(WebCoreWindRuleToSkFillType(rule));
}

void Path::MoveTo(const FloatPoint& point) {
  path_.moveTo(point.Data());
}

void Path::AddLineTo(const FloatPoint& point) {
  path_.lineTo(point.Data());
}

void Path::AddQuadCurveTo(const FloatPoint& cp, const FloatPoint& ep) {
  path_.quadTo(cp.Data(), ep.Data());
}

void Path::AddBezierCurveTo(const FloatPoint& p1,
                            const FloatPoint& p2,
                            const FloatPoint& ep) {
  path_.cubicTo(p1.Data(), p2.Data(), ep.Data());
}

void Path::AddArcTo(const FloatPoint& p1, const FloatPoint& p2, float radius) {
  path_.arcTo(p1.Data(), p2.Data(), WebCoreFloatToSkScalar(radius));
}

void Path::AddArcTo(const FloatPoint& p,
                    const FloatSize& r,
                    float x_rotate,
                    bool large_arc,
                    bool sweep) {
  path_.arcTo(WebCoreFloatToSkScalar(r.Width()),
              WebCoreFloatToSkScalar(r.Height()),
              WebCoreFloatToSkScalar(x_rotate),
              large_arc ? SkPath::kLarge_ArcSize : SkPath::kSmall_ArcSize,
              sweep ? SkPath::kCW_Direction : SkPath::kCCW_Direction,
              WebCoreFloatToSkScalar(p.X()), WebCoreFloatToSkScalar(p.Y()));
}

void Path::CloseSubpath() {
  path_.close();
}

void Path::AddEllipse(const FloatPoint& p,
                      float radius_x,
                      float radius_y,
                      float start_angle,
                      float end_angle,
                      bool anticlockwise) {
  DCHECK(EllipseIsRenderable(start_angle, end_angle));
  DCHECK_GE(start_angle, 0);
  DCHECK_LT(start_angle, twoPiFloat);
  DCHECK((anticlockwise && (start_angle - end_angle) >= 0) ||
         (!anticlockwise && (end_angle - start_angle) >= 0));

  SkScalar cx = WebCoreFloatToSkScalar(p.X());
  SkScalar cy = WebCoreFloatToSkScalar(p.Y());
  SkScalar radius_x_scalar = WebCoreFloatToSkScalar(radius_x);
  SkScalar radius_y_scalar = WebCoreFloatToSkScalar(radius_y);

  SkRect oval;
  oval.set(cx - radius_x_scalar, cy - radius_y_scalar, cx + radius_x_scalar,
           cy + radius_y_scalar);

  float sweep = end_angle - start_angle;
  SkScalar start_degrees = WebCoreFloatToSkScalar(start_angle * 180 / piFloat);
  SkScalar sweep_degrees = WebCoreFloatToSkScalar(sweep * 180 / piFloat);
  SkScalar s360 = SkIntToScalar(360);

  // We can't use SkPath::addOval(), because addOval() makes a new sub-path.
  // addOval() calls moveTo() and close() internally.

  // Use s180, not s360, because SkPath::arcTo(oval, angle, s360, false) draws
  // nothing.
  SkScalar s180 = SkIntToScalar(180);
  if (SkScalarNearlyEqual(sweep_degrees, s360)) {
    // SkPath::arcTo can't handle the sweepAngle that is equal to or greater
    // than 2Pi.
    path_.arcTo(oval, start_degrees, s180, false);
    path_.arcTo(oval, start_degrees + s180, s180, false);
    return;
  }
  if (SkScalarNearlyEqual(sweep_degrees, -s360)) {
    path_.arcTo(oval, start_degrees, -s180, false);
    path_.arcTo(oval, start_degrees - s180, -s180, false);
    return;
  }

  path_.arcTo(oval, start_degrees, sweep_degrees, false);
}

void Path::AddArc(const FloatPoint& p,
                  float radius,
                  float start_angle,
                  float end_angle,
                  bool anticlockwise) {
  AddEllipse(p, radius, radius, start_angle, end_angle, anticlockwise);
}

void Path::AddRect(const FloatRect& rect) {
  // Start at upper-left, add clock-wise.
  path_.addRect(rect, SkPath::kCW_Direction, 0);
}

void Path::AddEllipse(const FloatPoint& p,
                      float radius_x,
                      float radius_y,
                      float rotation,
                      float start_angle,
                      float end_angle,
                      bool anticlockwise) {
  DCHECK(EllipseIsRenderable(start_angle, end_angle));
  DCHECK_GE(start_angle, 0);
  DCHECK_LT(start_angle, twoPiFloat);
  DCHECK((anticlockwise && (start_angle - end_angle) >= 0) ||
         (!anticlockwise && (end_angle - start_angle) >= 0));

  if (!rotation) {
    AddEllipse(FloatPoint(p.X(), p.Y()), radius_x, radius_y, start_angle,
               end_angle, anticlockwise);
    return;
  }

  // Add an arc after the relevant transform.
  AffineTransform ellipse_transform =
      AffineTransform::Translation(p.X(), p.Y()).RotateRadians(rotation);
  DCHECK(ellipse_transform.IsInvertible());
  AffineTransform inverse_ellipse_transform = ellipse_transform.Inverse();
  Transform(inverse_ellipse_transform);
  AddEllipse(FloatPoint::Zero(), radius_x, radius_y, start_angle, end_angle,
             anticlockwise);
  Transform(ellipse_transform);
}

void Path::AddEllipse(const FloatRect& rect) {
  // Start at 3 o'clock, add clock-wise.
  path_.addOval(rect, SkPath::kCW_Direction, 1);
}

void Path::AddRoundedRect(const FloatRoundedRect& r) {
  AddRoundedRect(r.Rect(), r.GetRadii().TopLeft(), r.GetRadii().TopRight(),
                 r.GetRadii().BottomLeft(), r.GetRadii().BottomRight());
}

void Path::AddRoundedRect(const FloatRect& rect,
                          const FloatSize& rounding_radii) {
  if (rect.IsEmpty())
    return;

  FloatSize radius(rounding_radii);
  FloatSize half_size(rect.Width() / 2, rect.Height() / 2);

  // Apply the SVG corner radius constraints, per the rect section of the SVG
  // shapes spec: if one of rx,ry is negative, then the other corner radius
  // value is used. If both values are negative then rx = ry = 0. If rx is
  // greater than half of the width of the rectangle then set rx to half of the
  // width; ry is handled similarly.

  if (radius.Width() < 0)
    radius.SetWidth((radius.Height() < 0) ? 0 : radius.Height());

  if (radius.Height() < 0)
    radius.SetHeight(radius.Width());

  if (radius.Width() > half_size.Width())
    radius.SetWidth(half_size.Width());

  if (radius.Height() > half_size.Height())
    radius.SetHeight(half_size.Height());

  AddPathForRoundedRect(rect, radius, radius, radius, radius);
}

void Path::AddRoundedRect(const FloatRect& rect,
                          const FloatSize& top_left_radius,
                          const FloatSize& top_right_radius,
                          const FloatSize& bottom_left_radius,
                          const FloatSize& bottom_right_radius) {
  if (rect.IsEmpty())
    return;

  if (rect.Width() < top_left_radius.Width() + top_right_radius.Width() ||
      rect.Width() < bottom_left_radius.Width() + bottom_right_radius.Width() ||
      rect.Height() < top_left_radius.Height() + bottom_left_radius.Height() ||
      rect.Height() <
          top_right_radius.Height() + bottom_right_radius.Height()) {
    // If all the radii cannot be accommodated, return a rect.
    // FIXME: Is this an error scenario, given that it appears the code in
    // FloatRoundedRect::constrainRadii() should be always called first? Should
    // we assert that this code is not reached? This fallback is very bad, since
    // it means that radii that are just barely too big due to rounding or
    // snapping will get completely ignored.
    AddRect(rect);
    return;
  }

  AddPathForRoundedRect(rect, top_left_radius, top_right_radius,
                        bottom_left_radius, bottom_right_radius);
}

void Path::AddPathForRoundedRect(const FloatRect& rect,
                                 const FloatSize& top_left_radius,
                                 const FloatSize& top_right_radius,
                                 const FloatSize& bottom_left_radius,
                                 const FloatSize& bottom_right_radius) {
  // Start at upper-left (after corner radii), add clock-wise.
  path_.addRRect(FloatRoundedRect(rect, top_left_radius, top_right_radius,
                                  bottom_left_radius, bottom_right_radius),
                 SkPath::kCW_Direction, 0);
}

void Path::AddPath(const Path& src, const AffineTransform& transform) {
  path_.addPath(src.GetSkPath(), AffineTransformToSkMatrix(transform));
}

void Path::Translate(const FloatSize& size) {
  path_.offset(WebCoreFloatToSkScalar(size.Width()),
               WebCoreFloatToSkScalar(size.Height()));
}

bool Path::SubtractPath(const Path& other) {
  return Op(path_, other.path_, kDifference_SkPathOp, &path_);
}

bool Path::UnionPath(const Path& other) {
  return Op(path_, other.path_, kUnion_SkPathOp, &path_);
}

bool Path::IntersectPath(const Path& other) {
  return Op(path_, other.path_, kIntersect_SkPathOp, &path_);
}

bool EllipseIsRenderable(float start_angle, float end_angle) {
  return (std::abs(end_angle - start_angle) < twoPiFloat) ||
         WebCoreFloatNearlyEqual(std::abs(end_angle - start_angle), twoPiFloat);
}

}  // namespace blink

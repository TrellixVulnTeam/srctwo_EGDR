// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/geometry/rect_f.h"

#include <algorithm>

#if defined(OS_IOS)
#include <CoreGraphics/CoreGraphics.h>
#elif defined(OS_MACOSX)
#include <ApplicationServices/ApplicationServices.h>
#endif

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "build/build_config.h"
#include "ui/gfx/geometry/insets_f.h"
#include "ui/gfx/geometry/safe_integer_conversions.h"

namespace gfx {

static void AdjustAlongAxis(float dst_origin,
                            float dst_size,
                            float* origin,
                            float* size) {
  *size = std::min(dst_size, *size);
  if (*origin < dst_origin)
    *origin = dst_origin;
  else
    *origin = std::min(dst_origin + dst_size, *origin + *size) - *size;
}

#if defined(OS_MACOSX)
RectF::RectF(const CGRect& r)
    : origin_(r.origin.x, r.origin.y), size_(r.size.width, r.size.height) {
}

CGRect RectF::ToCGRect() const {
  return CGRectMake(x(), y(), width(), height());
}
#endif

void RectF::Inset(const InsetsF& insets) {
  Inset(insets.left(), insets.top(), insets.right(), insets.bottom());
}

void RectF::Inset(float left, float top, float right, float bottom) {
  origin_ += Vector2dF(left, top);
  set_width(std::max(width() - left - right, static_cast<float>(0)));
  set_height(std::max(height() - top - bottom, static_cast<float>(0)));
}

void RectF::Offset(float horizontal, float vertical) {
  origin_ += Vector2dF(horizontal, vertical);
}

void RectF::operator+=(const Vector2dF& offset) {
  origin_ += offset;
}

void RectF::operator-=(const Vector2dF& offset) {
  origin_ -= offset;
}

InsetsF RectF::InsetsFrom(const RectF& inner) const {
  return InsetsF(inner.y() - y(),
                 inner.x() - x(),
                 bottom() - inner.bottom(),
                 right() - inner.right());
}

bool RectF::operator<(const RectF& other) const {
  if (origin_ == other.origin_) {
    if (width() == other.width()) {
      return height() < other.height();
    } else {
      return width() < other.width();
    }
  } else {
    return origin_ < other.origin_;
  }
}

bool RectF::Contains(float point_x, float point_y) const {
  return (point_x >= x()) && (point_x < right()) && (point_y >= y()) &&
         (point_y < bottom());
}

bool RectF::Contains(const RectF& rect) const {
  return (rect.x() >= x() && rect.right() <= right() && rect.y() >= y() &&
          rect.bottom() <= bottom());
}

bool RectF::Intersects(const RectF& rect) const {
  return !(IsEmpty() || rect.IsEmpty() || rect.x() >= right() ||
           rect.right() <= x() || rect.y() >= bottom() || rect.bottom() <= y());
}

void RectF::Intersect(const RectF& rect) {
  if (IsEmpty() || rect.IsEmpty()) {
    SetRect(0, 0, 0, 0);
    return;
  }

  float rx = std::max(x(), rect.x());
  float ry = std::max(y(), rect.y());
  float rr = std::min(right(), rect.right());
  float rb = std::min(bottom(), rect.bottom());

  if (rx >= rr || ry >= rb)
    rx = ry = rr = rb = 0;  // non-intersecting

  SetRect(rx, ry, rr - rx, rb - ry);
}

void RectF::Union(const RectF& rect) {
  if (IsEmpty()) {
    *this = rect;
    return;
  }
  if (rect.IsEmpty())
    return;

  float rx = std::min(x(), rect.x());
  float ry = std::min(y(), rect.y());
  float rr = std::max(right(), rect.right());
  float rb = std::max(bottom(), rect.bottom());

  SetRect(rx, ry, rr - rx, rb - ry);
}

void RectF::Subtract(const RectF& rect) {
  if (!Intersects(rect))
    return;
  if (rect.Contains(*static_cast<const RectF*>(this))) {
    SetRect(0, 0, 0, 0);
    return;
  }

  float rx = x();
  float ry = y();
  float rr = right();
  float rb = bottom();

  if (rect.y() <= y() && rect.bottom() >= bottom()) {
    // complete intersection in the y-direction
    if (rect.x() <= x()) {
      rx = rect.right();
    } else if (rect.right() >= right()) {
      rr = rect.x();
    }
  } else if (rect.x() <= x() && rect.right() >= right()) {
    // complete intersection in the x-direction
    if (rect.y() <= y()) {
      ry = rect.bottom();
    } else if (rect.bottom() >= bottom()) {
      rb = rect.y();
    }
  }
  SetRect(rx, ry, rr - rx, rb - ry);
}

void RectF::AdjustToFit(const RectF& rect) {
  float new_x = x();
  float new_y = y();
  float new_width = width();
  float new_height = height();
  AdjustAlongAxis(rect.x(), rect.width(), &new_x, &new_width);
  AdjustAlongAxis(rect.y(), rect.height(), &new_y, &new_height);
  SetRect(new_x, new_y, new_width, new_height);
}

PointF RectF::CenterPoint() const {
  return PointF(x() + width() / 2, y() + height() / 2);
}

void RectF::ClampToCenteredSize(const SizeF& size) {
  float new_width = std::min(width(), size.width());
  float new_height = std::min(height(), size.height());
  float new_x = x() + (width() - new_width) / 2;
  float new_y = y() + (height() - new_height) / 2;
  SetRect(new_x, new_y, new_width, new_height);
}

void RectF::SplitVertically(RectF* left_half, RectF* right_half) const {
  DCHECK(left_half);
  DCHECK(right_half);

  left_half->SetRect(x(), y(), width() / 2, height());
  right_half->SetRect(
      left_half->right(), y(), width() - left_half->width(), height());
}

bool RectF::SharesEdgeWith(const RectF& rect) const {
  return (y() == rect.y() && height() == rect.height() &&
          (x() == rect.right() || right() == rect.x())) ||
         (x() == rect.x() && width() == rect.width() &&
          (y() == rect.bottom() || bottom() == rect.y()));
}

float RectF::ManhattanDistanceToPoint(const PointF& point) const {
  float x_distance =
      std::max<float>(0, std::max(x() - point.x(), point.x() - right()));
  float y_distance =
      std::max<float>(0, std::max(y() - point.y(), point.y() - bottom()));

  return x_distance + y_distance;
}

float RectF::ManhattanInternalDistance(const RectF& rect) const {
  RectF c(*this);
  c.Union(rect);

  static const float kEpsilon = std::numeric_limits<float>::epsilon();
  float x = std::max(0.f, c.width() - width() - rect.width() + kEpsilon);
  float y = std::max(0.f, c.height() - height() - rect.height() + kEpsilon);
  return x + y;
}

bool RectF::IsExpressibleAsRect() const {
  return IsExpressibleAsInt(x()) && IsExpressibleAsInt(y()) &&
      IsExpressibleAsInt(width()) && IsExpressibleAsInt(height()) &&
      IsExpressibleAsInt(right()) && IsExpressibleAsInt(bottom());
}

std::string RectF::ToString() const {
  return base::StringPrintf("%s %s",
                            origin().ToString().c_str(),
                            size().ToString().c_str());
}

RectF IntersectRects(const RectF& a, const RectF& b) {
  RectF result = a;
  result.Intersect(b);
  return result;
}

RectF UnionRects(const RectF& a, const RectF& b) {
  RectF result = a;
  result.Union(b);
  return result;
}

RectF SubtractRects(const RectF& a, const RectF& b) {
  RectF result = a;
  result.Subtract(b);
  return result;
}

RectF BoundingRect(const PointF& p1, const PointF& p2) {
  float rx = std::min(p1.x(), p2.x());
  float ry = std::min(p1.y(), p2.y());
  float rr = std::max(p1.x(), p2.x());
  float rb = std::max(p1.y(), p2.y());
  return RectF(rx, ry, rr - rx, rb - ry);
}

}  // namespace gfx

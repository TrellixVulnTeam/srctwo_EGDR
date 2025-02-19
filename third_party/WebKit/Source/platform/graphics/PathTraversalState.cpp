/*
 * Copyright (C) 2006, 2007 Eric Seidel <eric@webkit.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "platform/graphics/PathTraversalState.h"

#include "platform/wtf/MathExtras.h"
#include "platform/wtf/Vector.h"

namespace blink {

static inline FloatPoint MidPoint(const FloatPoint& first,
                                  const FloatPoint& second) {
  return FloatPoint((first.X() + second.X()) / 2.0f,
                    (first.Y() + second.Y()) / 2.0f);
}

static inline float DistanceLine(const FloatPoint& start,
                                 const FloatPoint& end) {
  return sqrtf((end.X() - start.X()) * (end.X() - start.X()) +
               (end.Y() - start.Y()) * (end.Y() - start.Y()));
}

struct QuadraticBezier {
  DISALLOW_NEW_EXCEPT_PLACEMENT_NEW();
  QuadraticBezier() {}
  QuadraticBezier(const FloatPoint& s, const FloatPoint& c, const FloatPoint& e)
      : start(s), control(c), end(e), split_depth(0) {}

  double MagnitudeSquared() const {
    return ((double)(start.Dot(start)) + (double)(control.Dot(control)) +
            (double)(end.Dot(end))) /
           9.0;
  }

  float ApproximateDistance() const {
    return DistanceLine(start, control) + DistanceLine(control, end);
  }

  void Split(QuadraticBezier& left, QuadraticBezier& right) const {
    left.control = MidPoint(start, control);
    right.control = MidPoint(control, end);

    FloatPoint left_control_to_right_control =
        MidPoint(left.control, right.control);
    left.end = left_control_to_right_control;
    right.start = left_control_to_right_control;

    left.start = start;
    right.end = end;

    left.split_depth = right.split_depth = split_depth + 1;
  }

  FloatPoint start;
  FloatPoint control;
  FloatPoint end;
  unsigned short split_depth;
};

struct CubicBezier {
  DISALLOW_NEW_EXCEPT_PLACEMENT_NEW();
  CubicBezier() {}
  CubicBezier(const FloatPoint& s,
              const FloatPoint& c1,
              const FloatPoint& c2,
              const FloatPoint& e)
      : start(s), control1(c1), control2(c2), end(e), split_depth(0) {}

  double MagnitudeSquared() const {
    return ((double)(start.Dot(start)) + (double)(control1.Dot(control1)) +
            (double)(control2.Dot(control2)) + (double)(end.Dot(end))) /
           16.0;
  }

  float ApproximateDistance() const {
    return DistanceLine(start, control1) + DistanceLine(control1, control2) +
           DistanceLine(control2, end);
  }

  void Split(CubicBezier& left, CubicBezier& right) const {
    FloatPoint start_to_control1 = MidPoint(control1, control2);

    left.start = start;
    left.control1 = MidPoint(start, control1);
    left.control2 = MidPoint(left.control1, start_to_control1);

    right.control2 = MidPoint(control2, end);
    right.control1 = MidPoint(right.control2, start_to_control1);
    right.end = end;

    FloatPoint left_control2_to_right_control1 =
        MidPoint(left.control2, right.control1);
    left.end = left_control2_to_right_control1;
    right.start = left_control2_to_right_control1;

    left.split_depth = right.split_depth = split_depth + 1;
  }

  FloatPoint start;
  FloatPoint control1;
  FloatPoint control2;
  FloatPoint end;
  unsigned short split_depth;
};

template <class CurveType>
static float CurveLength(PathTraversalState& traversal_state, CurveType curve) {
  static const unsigned short kCurveSplitDepthLimit = 20;
  static const double kPathSegmentLengthToleranceSquared = 1.e-16;

  double curve_scale_for_tolerance_squared = curve.MagnitudeSquared();
  if (curve_scale_for_tolerance_squared < kPathSegmentLengthToleranceSquared)
    return 0;

  Vector<CurveType> curve_stack;
  curve_stack.push_back(curve);

  float total_length = 0;
  do {
    float length = curve.ApproximateDistance();
    double length_discrepancy = length - DistanceLine(curve.start, curve.end);
    if ((length_discrepancy * length_discrepancy) /
                curve_scale_for_tolerance_squared >
            kPathSegmentLengthToleranceSquared &&
        curve.split_depth < kCurveSplitDepthLimit) {
      CurveType left_curve;
      CurveType right_curve;
      curve.Split(left_curve, right_curve);
      curve = left_curve;
      curve_stack.push_back(right_curve);
    } else {
      total_length += length;
      if (traversal_state.action_ ==
              PathTraversalState::kTraversalPointAtLength ||
          traversal_state.action_ ==
              PathTraversalState::kTraversalNormalAngleAtLength) {
        traversal_state.previous_ = curve.start;
        traversal_state.current_ = curve.end;
        if (traversal_state.total_length_ + total_length >
            traversal_state.desired_length_)
          return total_length;
      }
      curve = curve_stack.back();
      curve_stack.pop_back();
    }
  } while (!curve_stack.IsEmpty());

  return total_length;
}

PathTraversalState::PathTraversalState(PathTraversalAction action)
    : action_(action),
      success_(false),
      total_length_(0),
      desired_length_(0),
      normal_angle_(0) {}

float PathTraversalState::CloseSubpath() {
  float distance = DistanceLine(current_, start_);
  current_ = start_;
  return distance;
}

float PathTraversalState::MoveTo(const FloatPoint& point) {
  current_ = start_ = point;
  return 0;
}

float PathTraversalState::LineTo(const FloatPoint& point) {
  float distance = DistanceLine(current_, point);
  current_ = point;
  return distance;
}

float PathTraversalState::CubicBezierTo(const FloatPoint& new_control1,
                                        const FloatPoint& new_control2,
                                        const FloatPoint& new_end) {
  float distance = CurveLength<CubicBezier>(
      *this, CubicBezier(current_, new_control1, new_control2, new_end));

  if (action_ != kTraversalPointAtLength &&
      action_ != kTraversalNormalAngleAtLength)
    current_ = new_end;

  return distance;
}

void PathTraversalState::ProcessSegment() {
  if (action_ == kTraversalSegmentAtLength && total_length_ >= desired_length_)
    success_ = true;

  if ((action_ == kTraversalPointAtLength ||
       action_ == kTraversalNormalAngleAtLength) &&
      total_length_ >= desired_length_) {
    float slope = FloatPoint(current_ - previous_).SlopeAngleRadians();
    if (action_ == kTraversalPointAtLength) {
      float offset = desired_length_ - total_length_;
      current_.Move(offset * cosf(slope), offset * sinf(slope));
    } else {
      normal_angle_ = rad2deg(slope);
    }
    success_ = true;
  }
  previous_ = current_;
}

}  // namespace blink

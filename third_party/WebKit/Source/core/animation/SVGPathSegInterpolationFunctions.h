// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SVGPathSegInterpolationFunctions_h
#define SVGPathSegInterpolationFunctions_h

#include "core/animation/InterpolableValue.h"
#include "core/svg/SVGPathData.h"
#include <memory>

namespace blink {

struct PathCoordinates {
  double initial_x = 0;
  double initial_y = 0;
  double current_x = 0;
  double current_y = 0;
};

class SVGPathSegInterpolationFunctions {
  STATIC_ONLY(SVGPathSegInterpolationFunctions);

 public:
  static std::unique_ptr<InterpolableValue> ConsumePathSeg(
      const PathSegmentData&,
      PathCoordinates& current_coordinates);
  static PathSegmentData ConsumeInterpolablePathSeg(
      const InterpolableValue&,
      SVGPathSegType,
      PathCoordinates& current_coordinates);
};

}  // namespace blink

#endif  // SVGPathSegInterpolationFunctions_h

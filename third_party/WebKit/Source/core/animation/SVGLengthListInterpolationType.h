// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SVGLengthListInterpolationType_h
#define SVGLengthListInterpolationType_h

#include "core/SVGNames.h"
#include "core/animation/SVGInterpolationType.h"
#include "core/svg/SVGLength.h"

namespace blink {

enum class SVGLengthMode;

class SVGLengthListInterpolationType : public SVGInterpolationType {
 public:
  SVGLengthListInterpolationType(const QualifiedName& attribute)
      : SVGInterpolationType(attribute),
        unit_mode_(SVGLength::LengthModeForAnimatedLengthAttribute(attribute)),
        negative_values_forbidden_(
            SVGLength::NegativeValuesForbiddenForAnimatedLengthAttribute(
                attribute)) {}

 private:
  InterpolationValue MaybeConvertNeutral(const InterpolationValue& underlying,
                                         ConversionCheckers&) const final;
  InterpolationValue MaybeConvertSVGValue(
      const SVGPropertyBase& svg_value) const final;
  PairwiseInterpolationValue MaybeMergeSingles(
      InterpolationValue&& start,
      InterpolationValue&& end) const final;
  void Composite(UnderlyingValueOwner&,
                 double underlying_fraction,
                 const InterpolationValue&,
                 double interpolation_fraction) const final;
  SVGPropertyBase* AppliedSVGValue(const InterpolableValue&,
                                   const NonInterpolableValue*) const final;
  void Apply(const InterpolableValue&,
             const NonInterpolableValue*,
             InterpolationEnvironment&) const final;

  const SVGLengthMode unit_mode_;
  const bool negative_values_forbidden_;
};

}  // namespace blink

#endif  // SVGLengthListInterpolationType_h

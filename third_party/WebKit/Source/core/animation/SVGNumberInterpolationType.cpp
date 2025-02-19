// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/animation/SVGNumberInterpolationType.h"

#include "core/animation/InterpolationEnvironment.h"
#include "core/animation/StringKeyframe.h"
#include "core/svg/SVGNumber.h"
#include "core/svg/properties/SVGAnimatedProperty.h"

namespace blink {

InterpolationValue SVGNumberInterpolationType::MaybeConvertNeutral(
    const InterpolationValue&,
    ConversionCheckers&) const {
  return InterpolationValue(InterpolableNumber::Create(0));
}

InterpolationValue SVGNumberInterpolationType::MaybeConvertSVGValue(
    const SVGPropertyBase& svg_value) const {
  if (svg_value.GetType() != kAnimatedNumber)
    return nullptr;
  return InterpolationValue(
      InterpolableNumber::Create(ToSVGNumber(svg_value).Value()));
}

SVGPropertyBase* SVGNumberInterpolationType::AppliedSVGValue(
    const InterpolableValue& interpolable_value,
    const NonInterpolableValue*) const {
  double value = ToInterpolableNumber(interpolable_value).Value();
  return SVGNumber::Create(is_non_negative_ && value < 0 ? 0 : value);
}

}  // namespace blink

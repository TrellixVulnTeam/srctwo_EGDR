// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SVGLengthInterpolationType_h
#define SVGLengthInterpolationType_h

#include "core/animation/SVGInterpolationType.h"
#include "core/svg/SVGLength.h"
#include <memory>

namespace blink {

class SVGLengthContext;
enum class SVGLengthMode;

class SVGLengthInterpolationType : public SVGInterpolationType {
 public:
  SVGLengthInterpolationType(const QualifiedName& attribute)
      : SVGInterpolationType(attribute),
        unit_mode_(SVGLength::LengthModeForAnimatedLengthAttribute(attribute)),
        negative_values_forbidden_(
            SVGLength::NegativeValuesForbiddenForAnimatedLengthAttribute(
                attribute)) {}

  static std::unique_ptr<InterpolableValue> NeutralInterpolableValue();
  static InterpolationValue ConvertSVGLength(const SVGLength&);
  static SVGLength* ResolveInterpolableSVGLength(
      const InterpolableValue&,
      const SVGLengthContext&,
      SVGLengthMode,
      bool negative_values_forbidden);

 private:
  InterpolationValue MaybeConvertNeutral(const InterpolationValue& underlying,
                                         ConversionCheckers&) const final;
  InterpolationValue MaybeConvertSVGValue(
      const SVGPropertyBase& svg_value) const final;
  SVGPropertyBase* AppliedSVGValue(const InterpolableValue&,
                                   const NonInterpolableValue*) const final;
  void Apply(const InterpolableValue&,
             const NonInterpolableValue*,
             InterpolationEnvironment&) const final;

  const SVGLengthMode unit_mode_;
  const bool negative_values_forbidden_;
};

}  // namespace blink

#endif  // SVGLengthInterpolationType_h

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/animation/CSSFontSizeInterpolationType.h"

#include <memory>
#include "core/animation/LengthInterpolationFunctions.h"
#include "core/css/CSSIdentifierValue.h"
#include "core/css/resolver/StyleResolverState.h"
#include "platform/LengthFunctions.h"
#include "platform/fonts/FontDescription.h"
#include "platform/wtf/PtrUtil.h"

namespace blink {

namespace {

class IsMonospaceChecker : public CSSInterpolationType::CSSConversionChecker {
 public:
  static std::unique_ptr<IsMonospaceChecker> Create(bool is_monospace) {
    return WTF::WrapUnique(new IsMonospaceChecker(is_monospace));
  }

 private:
  IsMonospaceChecker(bool is_monospace) : is_monospace_(is_monospace) {}

  bool IsValid(const StyleResolverState& state,
               const InterpolationValue&) const final {
    return is_monospace_ == state.Style()->GetFontDescription().IsMonospace();
  }

  const bool is_monospace_;
};

class InheritedFontSizeChecker
    : public CSSInterpolationType::CSSConversionChecker {
 public:
  static std::unique_ptr<InheritedFontSizeChecker> Create(
      const FontDescription::Size& inherited_font_size) {
    return WTF::WrapUnique(new InheritedFontSizeChecker(inherited_font_size));
  }

 private:
  InheritedFontSizeChecker(const FontDescription::Size& inherited_font_size)
      : inherited_font_size_(inherited_font_size.value) {}

  bool IsValid(const StyleResolverState& state,
               const InterpolationValue&) const final {
    return inherited_font_size_ ==
           state.ParentFontDescription().GetSize().value;
  }

  const float inherited_font_size_;
};

InterpolationValue ConvertFontSize(float size) {
  return InterpolationValue(
      LengthInterpolationFunctions::CreateInterpolablePixels(size));
}

InterpolationValue MaybeConvertKeyword(
    CSSValueID value_id,
    const StyleResolverState& state,
    InterpolationType::ConversionCheckers& conversion_checkers) {
  if (FontSize::IsValidValueID(value_id)) {
    bool is_monospace = state.Style()->GetFontDescription().IsMonospace();
    conversion_checkers.push_back(IsMonospaceChecker::Create(is_monospace));
    return ConvertFontSize(state.GetFontBuilder().FontSizeForKeyword(
        FontSize::KeywordSize(value_id), is_monospace));
  }

  if (value_id != CSSValueSmaller && value_id != CSSValueLarger)
    return nullptr;

  const FontDescription::Size& inherited_font_size =
      state.ParentFontDescription().GetSize();
  conversion_checkers.push_back(
      InheritedFontSizeChecker::Create(inherited_font_size));
  if (value_id == CSSValueSmaller)
    return ConvertFontSize(
        FontDescription::SmallerSize(inherited_font_size).value);
  return ConvertFontSize(
      FontDescription::LargerSize(inherited_font_size).value);
}

}  // namespace

InterpolationValue CSSFontSizeInterpolationType::MaybeConvertNeutral(
    const InterpolationValue&,
    ConversionCheckers&) const {
  return InterpolationValue(
      LengthInterpolationFunctions::CreateNeutralInterpolableValue());
}

InterpolationValue CSSFontSizeInterpolationType::MaybeConvertInitial(
    const StyleResolverState& state,
    ConversionCheckers& conversion_checkers) const {
  return MaybeConvertKeyword(FontSize::InitialValueID(), state,
                             conversion_checkers);
}

InterpolationValue CSSFontSizeInterpolationType::MaybeConvertInherit(
    const StyleResolverState& state,
    ConversionCheckers& conversion_checkers) const {
  const FontDescription::Size& inherited_font_size =
      state.ParentFontDescription().GetSize();
  conversion_checkers.push_back(
      InheritedFontSizeChecker::Create(inherited_font_size));
  return ConvertFontSize(inherited_font_size.value);
}

InterpolationValue CSSFontSizeInterpolationType::MaybeConvertValue(
    const CSSValue& value,
    const StyleResolverState* state,
    ConversionCheckers& conversion_checkers) const {
  std::unique_ptr<InterpolableValue> result =
      LengthInterpolationFunctions::MaybeConvertCSSValue(value)
          .interpolable_value;
  if (result)
    return InterpolationValue(std::move(result));

  if (!value.IsIdentifierValue())
    return nullptr;

  DCHECK(state);
  return MaybeConvertKeyword(ToCSSIdentifierValue(value).GetValueID(), *state,
                             conversion_checkers);
}

InterpolationValue
CSSFontSizeInterpolationType::MaybeConvertStandardPropertyUnderlyingValue(
    const ComputedStyle& style) const {
  return ConvertFontSize(style.SpecifiedFontSize());
}

void CSSFontSizeInterpolationType::ApplyStandardPropertyValue(
    const InterpolableValue& interpolable_value,
    const NonInterpolableValue*,
    StyleResolverState& state) const {
  const FontDescription& parent_font = state.ParentFontDescription();
  Length font_size_length = LengthInterpolationFunctions::CreateLength(
      interpolable_value, nullptr, state.FontSizeConversionData(),
      kValueRangeNonNegative);
  float font_size =
      FloatValueForLength(font_size_length, parent_font.GetSize().value);
  state.GetFontBuilder().SetSize(FontDescription::Size(
      0, font_size,
      !font_size_length.IsPercentOrCalc() || parent_font.IsAbsoluteSize()));
}

}  // namespace blink

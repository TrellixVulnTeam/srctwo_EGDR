// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/animation/SVGNumberListInterpolationType.h"

#include "core/animation/InterpolationEnvironment.h"
#include "core/animation/UnderlyingLengthChecker.h"
#include "core/svg/SVGNumberList.h"
#include <memory>

namespace blink {

InterpolationValue SVGNumberListInterpolationType::MaybeConvertNeutral(
    const InterpolationValue& underlying,
    ConversionCheckers& conversion_checkers) const {
  size_t underlying_length =
      UnderlyingLengthChecker::GetUnderlyingLength(underlying);
  conversion_checkers.push_back(
      UnderlyingLengthChecker::Create(underlying_length));

  if (underlying_length == 0)
    return nullptr;

  std::unique_ptr<InterpolableList> result =
      InterpolableList::Create(underlying_length);
  for (size_t i = 0; i < underlying_length; i++)
    result->Set(i, InterpolableNumber::Create(0));
  return InterpolationValue(std::move(result));
}

InterpolationValue SVGNumberListInterpolationType::MaybeConvertSVGValue(
    const SVGPropertyBase& svg_value) const {
  if (svg_value.GetType() != kAnimatedNumberList)
    return nullptr;

  const SVGNumberList& number_list = ToSVGNumberList(svg_value);
  std::unique_ptr<InterpolableList> result =
      InterpolableList::Create(number_list.length());
  for (size_t i = 0; i < number_list.length(); i++)
    result->Set(i, InterpolableNumber::Create(number_list.at(i)->Value()));
  return InterpolationValue(std::move(result));
}

PairwiseInterpolationValue SVGNumberListInterpolationType::MaybeMergeSingles(
    InterpolationValue&& start,
    InterpolationValue&& end) const {
  size_t start_length = ToInterpolableList(*start.interpolable_value).length();
  size_t end_length = ToInterpolableList(*end.interpolable_value).length();
  if (start_length != end_length)
    return nullptr;
  return InterpolationType::MaybeMergeSingles(std::move(start), std::move(end));
}

static void PadWithZeroes(std::unique_ptr<InterpolableValue>& list_pointer,
                          size_t padded_length) {
  InterpolableList& list = ToInterpolableList(*list_pointer);

  if (list.length() >= padded_length)
    return;

  std::unique_ptr<InterpolableList> result =
      InterpolableList::Create(padded_length);
  size_t i = 0;
  for (; i < list.length(); i++)
    result->Set(i, std::move(list.GetMutable(i)));
  for (; i < padded_length; i++)
    result->Set(i, InterpolableNumber::Create(0));
  list_pointer = std::move(result);
}

void SVGNumberListInterpolationType::Composite(
    UnderlyingValueOwner& underlying_value_owner,
    double underlying_fraction,
    const InterpolationValue& value,
    double interpolation_fraction) const {
  const InterpolableList& list = ToInterpolableList(*value.interpolable_value);

  if (ToInterpolableList(*underlying_value_owner.Value().interpolable_value)
          .length() <= list.length())
    PadWithZeroes(underlying_value_owner.MutableValue().interpolable_value,
                  list.length());

  InterpolableList& underlying_list = ToInterpolableList(
      *underlying_value_owner.MutableValue().interpolable_value);

  DCHECK_GE(underlying_list.length(), list.length());
  size_t i = 0;
  for (; i < list.length(); i++)
    underlying_list.GetMutable(i)->ScaleAndAdd(underlying_fraction,
                                               *list.Get(i));
  for (; i < underlying_list.length(); i++)
    underlying_list.GetMutable(i)->Scale(underlying_fraction);
}

SVGPropertyBase* SVGNumberListInterpolationType::AppliedSVGValue(
    const InterpolableValue& interpolable_value,
    const NonInterpolableValue*) const {
  SVGNumberList* result = SVGNumberList::Create();
  const InterpolableList& list = ToInterpolableList(interpolable_value);
  for (size_t i = 0; i < list.length(); i++)
    result->Append(
        SVGNumber::Create(ToInterpolableNumber(list.Get(i))->Value()));
  return result;
}

}  // namespace blink

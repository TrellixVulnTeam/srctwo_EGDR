/*
 * Copyright (C) 2004, 2005, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
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

#include "core/svg/SVGNumberList.h"

#include "core/svg/SVGAnimationElement.h"
#include "core/svg/SVGParserUtilities.h"
#include "platform/wtf/text/StringBuilder.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

SVGNumberList::SVGNumberList() {}

SVGNumberList::~SVGNumberList() {}

String SVGNumberList::ValueAsString() const {
  StringBuilder builder;

  ConstIterator it = begin();
  ConstIterator it_end = end();
  if (it != it_end) {
    builder.Append(it->ValueAsString());
    ++it;

    for (; it != it_end; ++it) {
      builder.Append(' ');
      builder.Append(it->ValueAsString());
    }
  }

  return builder.ToString();
}

template <typename CharType>
SVGParsingError SVGNumberList::Parse(const CharType*& ptr,
                                     const CharType* end) {
  const CharType* list_start = ptr;
  while (ptr < end) {
    float number = 0;
    if (!ParseNumber(ptr, end, number))
      return SVGParsingError(SVGParseStatus::kExpectedNumber, ptr - list_start);
    Append(SVGNumber::Create(number));
  }
  return SVGParseStatus::kNoError;
}

SVGParsingError SVGNumberList::SetValueAsString(const String& value) {
  Clear();

  if (value.IsEmpty())
    return SVGParseStatus::kNoError;

  // Don't call |clear()| if an error is encountered. SVG policy is to use
  // valid items before error.
  // Spec: http://www.w3.org/TR/SVG/single-page.html#implnote-ErrorProcessing
  if (value.Is8Bit()) {
    const LChar* ptr = value.Characters8();
    const LChar* end = ptr + value.length();
    return Parse(ptr, end);
  }
  const UChar* ptr = value.Characters16();
  const UChar* end = ptr + value.length();
  return Parse(ptr, end);
}

void SVGNumberList::Add(SVGPropertyBase* other, SVGElement* context_element) {
  SVGNumberList* other_list = ToSVGNumberList(other);

  if (length() != other_list->length())
    return;

  for (size_t i = 0; i < length(); ++i)
    at(i)->SetValue(at(i)->Value() + other_list->at(i)->Value());
}

void SVGNumberList::CalculateAnimatedValue(
    SVGAnimationElement* animation_element,
    float percentage,
    unsigned repeat_count,
    SVGPropertyBase* from_value,
    SVGPropertyBase* to_value,
    SVGPropertyBase* to_at_end_of_duration_value,
    SVGElement* context_element) {
  SVGNumberList* from_list = ToSVGNumberList(from_value);
  SVGNumberList* to_list = ToSVGNumberList(to_value);
  SVGNumberList* to_at_end_of_duration_list =
      ToSVGNumberList(to_at_end_of_duration_value);

  size_t from_list_size = from_list->length();
  size_t to_list_size = to_list->length();
  size_t to_at_end_of_duration_list_size = to_at_end_of_duration_list->length();

  if (!AdjustFromToListValues(from_list, to_list, percentage,
                              animation_element->GetAnimationMode()))
    return;

  for (size_t i = 0; i < to_list_size; ++i) {
    float effective_from = from_list_size ? from_list->at(i)->Value() : 0;
    float effective_to = to_list_size ? to_list->at(i)->Value() : 0;
    float effective_to_at_end = i < to_at_end_of_duration_list_size
                                    ? to_at_end_of_duration_list->at(i)->Value()
                                    : 0;

    float animated = at(i)->Value();
    animation_element->AnimateAdditiveNumber(percentage, repeat_count,
                                             effective_from, effective_to,
                                             effective_to_at_end, animated);
    at(i)->SetValue(animated);
  }
}

float SVGNumberList::CalculateDistance(SVGPropertyBase* to, SVGElement*) {
  // FIXME: Distance calculation is not possible for SVGNumberList right now. We
  // need the distance for every single value.
  return -1;
}

Vector<float> SVGNumberList::ToFloatVector() const {
  Vector<float> vec;
  vec.ReserveInitialCapacity(length());
  for (size_t i = 0; i < length(); ++i)
    vec.UncheckedAppend(at(i)->Value());
  return vec;
}

}  // namespace blink

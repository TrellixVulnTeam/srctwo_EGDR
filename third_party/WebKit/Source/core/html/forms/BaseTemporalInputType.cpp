/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "core/html/forms/BaseTemporalInputType.h"

#include <limits>
#include "core/html/HTMLInputElement.h"
#include "core/html/forms/ChooserOnlyTemporalInputTypeView.h"
#include "core/html/forms/MultipleFieldsTemporalInputTypeView.h"
#include "platform/text/PlatformLocale.h"
#include "platform/wtf/CurrentTime.h"
#include "platform/wtf/DateMath.h"
#include "platform/wtf/MathExtras.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

using blink::WebLocalizedString;
using namespace HTMLNames;

static const int kMsecPerMinute = 60 * 1000;
static const int kMsecPerSecond = 1000;

String BaseTemporalInputType::BadInputText() const {
  return GetLocale().QueryString(
      WebLocalizedString::kValidationBadInputForDateTime);
}

InputTypeView* BaseTemporalInputType::CreateView() {
  if (RuntimeEnabledFeatures::InputMultipleFieldsUIEnabled())
    return MultipleFieldsTemporalInputTypeView::Create(GetElement(), *this);
  return ChooserOnlyTemporalInputTypeView::Create(GetElement(), *this);
}

InputType::ValueMode BaseTemporalInputType::GetValueMode() const {
  return ValueMode::kValue;
}

double BaseTemporalInputType::ValueAsDate() const {
  return ValueAsDouble();
}

void BaseTemporalInputType::SetValueAsDate(double value,
                                           ExceptionState&) const {
  GetElement().setValue(SerializeWithMilliseconds(value));
}

double BaseTemporalInputType::ValueAsDouble() const {
  const Decimal value = ParseToNumber(GetElement().value(), Decimal::Nan());
  return value.IsFinite() ? value.ToDouble()
                          : DateComponents::InvalidMilliseconds();
}

void BaseTemporalInputType::SetValueAsDouble(
    double new_value,
    TextFieldEventBehavior event_behavior,
    ExceptionState& exception_state) const {
  SetValueAsDecimal(Decimal::FromDouble(new_value), event_behavior,
                    exception_state);
}

bool BaseTemporalInputType::TypeMismatchFor(const String& value) const {
  return !value.IsEmpty() && !ParseToDateComponents(value, 0);
}

bool BaseTemporalInputType::TypeMismatch() const {
  return TypeMismatchFor(GetElement().value());
}

String BaseTemporalInputType::RangeOverflowText(const Decimal& maximum) const {
  return GetLocale().QueryString(
      WebLocalizedString::kValidationRangeOverflowDateTime,
      LocalizeValue(Serialize(maximum)));
}

String BaseTemporalInputType::RangeUnderflowText(const Decimal& minimum) const {
  return GetLocale().QueryString(
      WebLocalizedString::kValidationRangeUnderflowDateTime,
      LocalizeValue(Serialize(minimum)));
}

Decimal BaseTemporalInputType::DefaultValueForStepUp() const {
  return Decimal::FromDouble(ConvertToLocalTime(CurrentTimeMS()));
}

bool BaseTemporalInputType::IsSteppable() const {
  return true;
}

Decimal BaseTemporalInputType::ParseToNumber(
    const String& source,
    const Decimal& default_value) const {
  DateComponents date;
  if (!ParseToDateComponents(source, &date))
    return default_value;
  double msec = date.MillisecondsSinceEpoch();
  DCHECK(std::isfinite(msec));
  return Decimal::FromDouble(msec);
}

bool BaseTemporalInputType::ParseToDateComponents(const String& source,
                                                  DateComponents* out) const {
  if (source.IsEmpty())
    return false;
  DateComponents ignored_result;
  if (!out)
    out = &ignored_result;
  return ParseToDateComponentsInternal(source, out);
}

String BaseTemporalInputType::Serialize(const Decimal& value) const {
  if (!value.IsFinite())
    return String();
  DateComponents date;
  if (!SetMillisecondToDateComponents(value.ToDouble(), &date))
    return String();
  return SerializeWithComponents(date);
}

String BaseTemporalInputType::SerializeWithComponents(
    const DateComponents& date) const {
  Decimal step;
  if (!GetElement().GetAllowedValueStep(&step))
    return date.ToString();
  if (step.Remainder(kMsecPerMinute).IsZero())
    return date.ToString(DateComponents::kNone);
  if (step.Remainder(kMsecPerSecond).IsZero())
    return date.ToString(DateComponents::kSecond);
  return date.ToString(DateComponents::kMillisecond);
}

String BaseTemporalInputType::SerializeWithMilliseconds(double value) const {
  return Serialize(Decimal::FromDouble(value));
}

String BaseTemporalInputType::LocalizeValue(
    const String& proposed_value) const {
  DateComponents date;
  if (!ParseToDateComponents(proposed_value, &date))
    return proposed_value;

  String localized = GetElement().GetLocale().FormatDateTime(date);
  return localized.IsEmpty() ? proposed_value : localized;
}

String BaseTemporalInputType::VisibleValue() const {
  return LocalizeValue(GetElement().value());
}

String BaseTemporalInputType::SanitizeValue(
    const String& proposed_value) const {
  return TypeMismatchFor(proposed_value) ? g_empty_string : proposed_value;
}

bool BaseTemporalInputType::SupportsReadOnly() const {
  return true;
}

bool BaseTemporalInputType::ShouldRespectListAttribute() {
  return true;
}

bool BaseTemporalInputType::ValueMissing(const String& value) const {
  return GetElement().IsRequired() && value.IsEmpty();
}

bool BaseTemporalInputType::ShouldShowFocusRingOnMouseFocus() const {
  return true;
}

bool BaseTemporalInputType::ShouldHaveSecondField(
    const DateComponents& date) const {
  StepRange step_range = CreateStepRange(kAnyIsDefaultStep);
  return date.Second() || date.Millisecond() ||
         !step_range.Minimum()
              .Remainder(static_cast<int>(kMsPerMinute))
              .IsZero() ||
         !step_range.Step().Remainder(static_cast<int>(kMsPerMinute)).IsZero();
}

}  // namespace blink

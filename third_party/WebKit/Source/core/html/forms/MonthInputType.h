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

#ifndef MonthInputType_h
#define MonthInputType_h

#include "core/html/forms/BaseTemporalInputType.h"

namespace blink {

class MonthInputType final : public BaseTemporalInputType {
 public:
  static InputType* Create(HTMLInputElement&);

 private:
  explicit MonthInputType(HTMLInputElement& element)
      : BaseTemporalInputType(element) {}

  void CountUsage() override;
  const AtomicString& FormControlType() const override;
  double ValueAsDate() const override;
  String SerializeWithMilliseconds(double) const override;
  Decimal ParseToNumber(const String&, const Decimal&) const override;
  Decimal DefaultValueForStepUp() const override;
  StepRange CreateStepRange(AnyStepHandling) const override;
  bool ParseToDateComponentsInternal(const String&,
                                     DateComponents*) const override;
  bool SetMillisecondToDateComponents(double, DateComponents*) const override;
  bool CanSetSuggestedValue() override;
  void WarnIfValueIsInvalid(const String&) const override;

  // BaseTemporalInputType functions
  String FormatDateTimeFieldsState(const DateTimeFieldsState&) const override;
  void SetupLayoutParameters(DateTimeEditElement::LayoutParameters&,
                             const DateComponents&) const override;
  bool IsValidFormat(bool has_year,
                     bool has_month,
                     bool has_week,
                     bool has_day,
                     bool has_ampm,
                     bool has_hour,
                     bool has_minute,
                     bool has_second) const override;
};

}  // namespace blink

#endif  // MonthInputType_h

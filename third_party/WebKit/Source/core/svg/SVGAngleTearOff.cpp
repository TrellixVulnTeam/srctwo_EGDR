/*
 * Copyright (C) 2014 Google Inc. All rights reserved.
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

#include "core/svg/SVGAngleTearOff.h"

#include "bindings/core/v8/ExceptionState.h"
#include "core/dom/ExceptionCode.h"
#include "core/svg/SVGElement.h"

namespace blink {

SVGAngleTearOff::SVGAngleTearOff(SVGAngle* target_property,
                                 SVGElement* context_element,
                                 PropertyIsAnimValType property_is_anim_val,
                                 const QualifiedName& attribute_name)
    : SVGPropertyTearOff<SVGAngle>(target_property,
                                   context_element,
                                   property_is_anim_val,
                                   attribute_name) {}

SVGAngleTearOff::~SVGAngleTearOff() {}

void SVGAngleTearOff::setValue(float value, ExceptionState& exception_state) {
  if (IsImmutable()) {
    ThrowReadOnly(exception_state);
    return;
  }
  Target()->SetValue(value);
  CommitChange();
}

void SVGAngleTearOff::setValueInSpecifiedUnits(
    float value,
    ExceptionState& exception_state) {
  if (IsImmutable()) {
    ThrowReadOnly(exception_state);
    return;
  }
  Target()->SetValueInSpecifiedUnits(value);
  CommitChange();
}

void SVGAngleTearOff::newValueSpecifiedUnits(unsigned short unit_type,
                                             float value_in_specified_units,
                                             ExceptionState& exception_state) {
  if (IsImmutable()) {
    ThrowReadOnly(exception_state);
    return;
  }
  if (unit_type == SVGAngle::kSvgAngletypeUnknown ||
      unit_type > SVGAngle::kSvgAngletypeGrad) {
    exception_state.ThrowDOMException(
        kNotSupportedError, "Cannot set value with unknown or invalid units (" +
                                String::Number(unit_type) + ").");
    return;
  }
  Target()->NewValueSpecifiedUnits(
      static_cast<SVGAngle::SVGAngleType>(unit_type), value_in_specified_units);
  CommitChange();
}

void SVGAngleTearOff::convertToSpecifiedUnits(unsigned short unit_type,
                                              ExceptionState& exception_state) {
  if (IsImmutable()) {
    ThrowReadOnly(exception_state);
    return;
  }
  if (unit_type == SVGAngle::kSvgAngletypeUnknown ||
      unit_type > SVGAngle::kSvgAngletypeGrad) {
    exception_state.ThrowDOMException(
        kNotSupportedError, "Cannot convert to unknown or invalid units (" +
                                String::Number(unit_type) + ").");
    return;
  }
  if (Target()->UnitType() == SVGAngle::kSvgAngletypeUnknown) {
    exception_state.ThrowDOMException(
        kNotSupportedError, "Cannot convert from unknown or invalid units.");
    return;
  }
  Target()->ConvertToSpecifiedUnits(
      static_cast<SVGAngle::SVGAngleType>(unit_type));
  CommitChange();
}

void SVGAngleTearOff::setValueAsString(const String& value,
                                       ExceptionState& exception_state) {
  if (IsImmutable()) {
    ThrowReadOnly(exception_state);
    return;
  }
  String old_value = Target()->ValueAsString();
  SVGParsingError status = Target()->SetValueAsString(value);
  if (status == SVGParseStatus::kNoError && !HasExposedAngleUnit()) {
    Target()->SetValueAsString(old_value);  // rollback to old value
    status = SVGParseStatus::kParsingFailed;
  }
  if (status != SVGParseStatus::kNoError) {
    exception_state.ThrowDOMException(
        kSyntaxError, "The value provided ('" + value + "') is invalid.");
    return;
  }
  CommitChange();
}

SVGAngleTearOff* SVGAngleTearOff::CreateDetached() {
  return Create(SVGAngle::Create(), nullptr, kPropertyIsNotAnimVal,
                QualifiedName::Null());
}

DEFINE_TRACE_WRAPPERS(SVGAngleTearOff) {
  SVGPropertyTearOff<SVGAngle>::TraceWrappers(visitor);
  ScriptWrappable::TraceWrappers(visitor);
}

}  // namespace blink

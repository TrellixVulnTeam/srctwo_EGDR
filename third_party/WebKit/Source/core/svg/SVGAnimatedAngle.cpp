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

#include "core/svg/SVGAnimatedAngle.h"

#include "core/SVGNames.h"

namespace blink {

SVGAnimatedAngle::SVGAnimatedAngle(SVGElement* context_element)
    : SVGAnimatedProperty<SVGAngle>(context_element,
                                    SVGNames::orientAttr,
                                    SVGAngle::Create()),
      orient_type_(SVGAnimatedEnumeration<SVGMarkerOrientType>::Create(
          context_element,
          SVGNames::orientAttr,
          BaseValue()->OrientType())) {}

SVGAnimatedAngle::~SVGAnimatedAngle() {}

DEFINE_TRACE(SVGAnimatedAngle) {
  visitor->Trace(orient_type_);
  SVGAnimatedProperty<SVGAngle>::Trace(visitor);
}

DEFINE_TRACE_WRAPPERS(SVGAnimatedAngle) {
  SVGAnimatedProperty<SVGAngle>::TraceWrappers(visitor);
  ScriptWrappable::TraceWrappers(visitor);
}

bool SVGAnimatedAngle::NeedsSynchronizeAttribute() {
  return orient_type_->NeedsSynchronizeAttribute() ||
         SVGAnimatedProperty<SVGAngle>::NeedsSynchronizeAttribute();
}

void SVGAnimatedAngle::SynchronizeAttribute() {
  // If the current value is not an <angle> we synchronize the value of the
  // wrapped enumeration.
  if (orient_type_->CurrentValue()->EnumValue() != kSVGMarkerOrientAngle) {
    orient_type_->SynchronizeAttribute();
    return;
  }
  SVGAnimatedProperty<SVGAngle>::SynchronizeAttribute();
}

void SVGAnimatedAngle::SetAnimatedValue(SVGPropertyBase* value) {
  SVGAnimatedProperty<SVGAngle>::SetAnimatedValue(value);
  orient_type_->SetAnimatedValue(CurrentValue()->OrientType());
}

void SVGAnimatedAngle::AnimationEnded() {
  SVGAnimatedProperty<SVGAngle>::AnimationEnded();
  orient_type_->AnimationEnded();
}

}  // namespace blink

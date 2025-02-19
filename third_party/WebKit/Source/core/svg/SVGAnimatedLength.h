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

#ifndef SVGAnimatedLength_h
#define SVGAnimatedLength_h

#include "core/svg/SVGLengthTearOff.h"
#include "core/svg/properties/SVGAnimatedProperty.h"
#include "platform/bindings/ScriptWrappable.h"

namespace blink {

class SVGAnimatedLength : public SVGAnimatedProperty<SVGLength>,
                          public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static SVGAnimatedLength* Create(
      SVGElement* context_element,
      const QualifiedName& attribute_name,
      SVGLength* initial_value,
      CSSPropertyID css_property_id = CSSPropertyInvalid) {
    return new SVGAnimatedLength(context_element, attribute_name, initial_value,
                                 css_property_id);
  }

  void SetDefaultValueAsString(const String&);
  SVGParsingError SetBaseValueAsString(const String&) override;

  const CSSValue* CssValue() const {
    return &CurrentValue()->AsCSSPrimitiveValue();
  }

  DECLARE_VIRTUAL_TRACE_WRAPPERS();

 protected:
  SVGAnimatedLength(SVGElement* context_element,
                    const QualifiedName& attribute_name,
                    SVGLength* initial_value,
                    CSSPropertyID css_property_id = CSSPropertyInvalid)
      : SVGAnimatedProperty<SVGLength>(context_element,
                                       attribute_name,
                                       initial_value,
                                       css_property_id) {}
};

}  // namespace blink

#endif  // SVGAnimatedLength_h

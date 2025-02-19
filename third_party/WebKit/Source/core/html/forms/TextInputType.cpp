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

#include "core/html/forms/TextInputType.h"

#include "core/InputTypeNames.h"
#include "core/html/HTMLInputElement.h"

namespace blink {

using namespace HTMLNames;

InputType* TextInputType::Create(HTMLInputElement& element) {
  return new TextInputType(element);
}

void TextInputType::CountUsage() {
  CountUsageIfVisible(WebFeature::kInputTypeText);
  if (GetElement().FastHasAttribute(maxlengthAttr))
    CountUsageIfVisible(WebFeature::kInputTypeTextMaxLength);
  const AtomicString& type = GetElement().FastGetAttribute(typeAttr);
  if (DeprecatedEqualIgnoringCase(type, InputTypeNames::datetime))
    CountUsageIfVisible(WebFeature::kInputTypeDateTimeFallback);
  else if (DeprecatedEqualIgnoringCase(type, InputTypeNames::week))
    CountUsageIfVisible(WebFeature::kInputTypeWeekFallback);
}

const AtomicString& TextInputType::FormControlType() const {
  return InputTypeNames::text;
}

bool TextInputType::SupportsInputModeAttribute() const {
  return true;
}

const AtomicString& TextInputType::DefaultAutocapitalize() const {
  DEFINE_STATIC_LOCAL(const AtomicString, sentences, ("sentences"));
  return sentences;
}

}  // namespace blink

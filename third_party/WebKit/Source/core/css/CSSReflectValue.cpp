/*
 * Copyright (C) 2008 Apple Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "core/css/CSSReflectValue.h"

#include "core/css/CSSIdentifierValue.h"
#include "core/css/CSSPrimitiveValue.h"

namespace blink {

String CSSReflectValue::CustomCSSText() const {
  if (mask_)
    return direction_->CssText() + ' ' + offset_->CssText() + ' ' +
           mask_->CssText();
  return direction_->CssText() + ' ' + offset_->CssText();
}

bool CSSReflectValue::Equals(const CSSReflectValue& other) const {
  return direction_ == other.direction_ &&
         DataEquivalent(offset_, other.offset_) &&
         DataEquivalent(mask_, other.mask_);
}

DEFINE_TRACE_AFTER_DISPATCH(CSSReflectValue) {
  visitor->Trace(direction_);
  visitor->Trace(offset_);
  visitor->Trace(mask_);
  CSSValue::TraceAfterDispatch(visitor);
}

}  // namespace blink

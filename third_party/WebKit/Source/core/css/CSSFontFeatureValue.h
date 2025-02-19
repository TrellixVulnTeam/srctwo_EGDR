/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
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

#ifndef CSSFontFeatureValue_h
#define CSSFontFeatureValue_h

#include "core/css/CSSValue.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class CSSFontFeatureValue : public CSSValue {
 public:
  static CSSFontFeatureValue* Create(const AtomicString& tag, int value) {
    return new CSSFontFeatureValue(tag, value);
  }

  const AtomicString& Tag() const { return tag_; }
  int Value() const { return value_; }
  String CustomCSSText() const;

  bool Equals(const CSSFontFeatureValue&) const;

  DEFINE_INLINE_TRACE_AFTER_DISPATCH() {
    CSSValue::TraceAfterDispatch(visitor);
  }

 private:
  CSSFontFeatureValue(const AtomicString& tag, int value);

  AtomicString tag_;
  const int value_;
};

DEFINE_CSS_VALUE_TYPE_CASTS(CSSFontFeatureValue, IsFontFeatureValue());

}  // namespace blink

#endif

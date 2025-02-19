// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CSSPendingSubstitutionValue_h
#define CSSPendingSubstitutionValue_h

#include "core/CSSPropertyNames.h"
#include "core/css/CSSValue.h"
#include "core/css/CSSVariableReferenceValue.h"

namespace blink {

class CSSPendingSubstitutionValue : public CSSValue {
 public:
  static CSSPendingSubstitutionValue* Create(
      CSSPropertyID shorthand_property_id,
      CSSVariableReferenceValue* shorthand_value) {
    return new CSSPendingSubstitutionValue(shorthand_property_id,
                                           shorthand_value);
  }

  CSSVariableReferenceValue* ShorthandValue() const {
    return shorthand_value_.Get();
  }

  CSSPropertyID ShorthandPropertyId() const { return shorthand_property_id_; }

  bool Equals(const CSSPendingSubstitutionValue& other) const {
    return shorthand_value_ == other.shorthand_value_;
  }
  String CustomCSSText() const;

  DECLARE_TRACE_AFTER_DISPATCH();

 private:
  CSSPendingSubstitutionValue(CSSPropertyID shorthand_property_id,
                              CSSVariableReferenceValue* shorthand_value)
      : CSSValue(kPendingSubstitutionValueClass),
        shorthand_property_id_(shorthand_property_id),
        shorthand_value_(shorthand_value) {}

  CSSPropertyID shorthand_property_id_;
  Member<CSSVariableReferenceValue> shorthand_value_;
};

DEFINE_CSS_VALUE_TYPE_CASTS(CSSPendingSubstitutionValue,
                            IsPendingSubstitutionValue());

}  // namespace blink

#endif  // CSSPendingSubstitutionValue_h

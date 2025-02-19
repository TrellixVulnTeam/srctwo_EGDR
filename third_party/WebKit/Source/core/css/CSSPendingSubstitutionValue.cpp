// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/css/CSSPendingSubstitutionValue.h"

namespace blink {

DEFINE_TRACE_AFTER_DISPATCH(CSSPendingSubstitutionValue) {
  CSSValue::TraceAfterDispatch(visitor);
  visitor->Trace(shorthand_value_);
}

String CSSPendingSubstitutionValue::CustomCSSText() const {
  return "";
}

}  // namespace blink

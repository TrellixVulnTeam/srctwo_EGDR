// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/css/properties/CSSPropertyAPITextShadow.h"

#include "core/css/parser/CSSParserContext.h"
#include "core/css/properties/CSSPropertyBoxShadowUtils.h"

namespace blink {

const CSSValue* CSSPropertyAPITextShadow::ParseSingleValue(
    CSSPropertyID,
    CSSParserTokenRange& range,
    const CSSParserContext& context,
    const CSSParserLocalContext&) const {
  return CSSPropertyBoxShadowUtils::ConsumeShadow(range, context.Mode(),
                                                  AllowInsetAndSpread::kForbid);
}

}  // namespace blink

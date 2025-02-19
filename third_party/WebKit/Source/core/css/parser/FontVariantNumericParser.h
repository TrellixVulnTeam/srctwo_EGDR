// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FontVariantNumericParser_h
#define FontVariantNumericParser_h

#include "core/css/CSSValueList.h"
#include "core/css/parser/CSSParserTokenRange.h"
#include "core/css/parser/CSSPropertyParserHelpers.h"

namespace blink {

class FontVariantNumericParser {
  STACK_ALLOCATED();

 public:
  FontVariantNumericParser()
      : saw_numeric_figure_value_(false),
        saw_numeric_spacing_value_(false),
        saw_numeric_fraction_value_(false),
        saw_ordinal_value_(false),
        saw_slashed_zero_value_(false),
        result_(CSSValueList::CreateSpaceSeparated()) {}

  enum class ParseResult { kConsumedValue, kDisallowedValue, kUnknownValue };

  ParseResult ConsumeNumeric(CSSParserTokenRange& range) {
    CSSValueID value_id = range.Peek().Id();
    switch (value_id) {
      case CSSValueLiningNums:
      case CSSValueOldstyleNums:
        if (saw_numeric_figure_value_)
          return ParseResult::kDisallowedValue;
        saw_numeric_figure_value_ = true;
        break;
      case CSSValueProportionalNums:
      case CSSValueTabularNums:
        if (saw_numeric_spacing_value_)
          return ParseResult::kDisallowedValue;
        saw_numeric_spacing_value_ = true;
        break;
      case CSSValueDiagonalFractions:
      case CSSValueStackedFractions:
        if (saw_numeric_fraction_value_)
          return ParseResult::kDisallowedValue;
        saw_numeric_fraction_value_ = true;
        break;
      case CSSValueOrdinal:
        if (saw_ordinal_value_)
          return ParseResult::kDisallowedValue;
        saw_ordinal_value_ = true;
        break;
      case CSSValueSlashedZero:
        if (saw_slashed_zero_value_)
          return ParseResult::kDisallowedValue;
        saw_slashed_zero_value_ = true;
        break;
      default:
        return ParseResult::kUnknownValue;
    }
    result_->Append(*CSSPropertyParserHelpers::ConsumeIdent(range));
    return ParseResult::kConsumedValue;
  }

  CSSValue* FinalizeValue() {
    if (!result_->length())
      return CSSIdentifierValue::Create(CSSValueNormal);
    return result_.Release();
  }

 private:
  bool saw_numeric_figure_value_;
  bool saw_numeric_spacing_value_;
  bool saw_numeric_fraction_value_;
  bool saw_ordinal_value_;
  bool saw_slashed_zero_value_;
  Member<CSSValueList> result_;
};

}  // namespace blink

#endif  // FontVariantNumericParser_h

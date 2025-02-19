// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/css/parser/CSSTokenizerInputStream.h"

#include "core/html/parser/HTMLParserIdioms.h"
#include "platform/wtf/text/StringToNumber.h"

namespace blink {

CSSTokenizerInputStream::CSSTokenizerInputStream(const String& input)
    : offset_(0), string_length_(input.length()), string_(input.Impl()) {}

void CSSTokenizerInputStream::AdvanceUntilNonWhitespace() {
  // Using HTML space here rather than CSS space since we don't do preprocessing
  if (string_->Is8Bit()) {
    const LChar* characters = string_->Characters8();
    while (offset_ < string_length_ && IsHTMLSpace(characters[offset_]))
      ++offset_;
  } else {
    const UChar* characters = string_->Characters16();
    while (offset_ < string_length_ && IsHTMLSpace(characters[offset_]))
      ++offset_;
  }
}

double CSSTokenizerInputStream::GetDouble(unsigned start, unsigned end) const {
  DCHECK(start <= end && ((offset_ + end) <= string_length_));
  bool is_result_ok = false;
  double result = 0.0;
  if (start < end) {
    if (string_->Is8Bit())
      result = CharactersToDouble(string_->Characters8() + offset_ + start,
                                  end - start, &is_result_ok);
    else
      result = CharactersToDouble(string_->Characters16() + offset_ + start,
                                  end - start, &is_result_ok);
  }
  // FIXME: It looks like callers ensure we have a valid number
  return is_result_ok ? result : 0.0;
}

}  // namespace blink

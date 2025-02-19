// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CSSTokenizerInputStream_h
#define CSSTokenizerInputStream_h

#include "platform/wtf/text/StringView.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class CSSTokenizerInputStream {
  WTF_MAKE_NONCOPYABLE(CSSTokenizerInputStream);
  USING_FAST_MALLOC(CSSTokenizerInputStream);

 public:
  explicit CSSTokenizerInputStream(const String& input);

  // Gets the char in the stream replacing NUL characters with a unicode
  // replacement character. Will return (NUL) kEndOfFileMarker when at the
  // end of the stream.
  UChar NextInputChar() const {
    if (offset_ >= string_length_)
      return '\0';
    UChar result = (*string_)[offset_];
    return result ? result : 0xFFFD;
  }

  // Gets the char at lookaheadOffset from the current stream position. Will
  // return NUL (kEndOfFileMarker) if the stream position is at the end.
  // NOTE: This may *also* return NUL if there's one in the input! Never
  // compare the return value to '\0'.
  UChar PeekWithoutReplacement(unsigned lookahead_offset) const {
    if ((offset_ + lookahead_offset) >= string_length_)
      return '\0';
    return (*string_)[offset_ + lookahead_offset];
  }

  void Advance(unsigned offset = 1) { offset_ += offset; }
  void PushBack(UChar cc) {
    --offset_;
    DCHECK(NextInputChar() == cc);
  }

  double GetDouble(unsigned start, unsigned end) const;

  template <bool characterPredicate(UChar)>
  unsigned SkipWhilePredicate(unsigned offset) {
    if (string_->Is8Bit()) {
      const LChar* characters8 = string_->Characters8();
      while ((offset_ + offset) < string_length_ &&
             characterPredicate(characters8[offset_ + offset]))
        ++offset;
    } else {
      const UChar* characters16 = string_->Characters16();
      while ((offset_ + offset) < string_length_ &&
             characterPredicate(characters16[offset_ + offset]))
        ++offset;
    }
    return offset;
  }

  void AdvanceUntilNonWhitespace();

  unsigned length() const { return string_length_; }
  unsigned Offset() const { return std::min(offset_, string_length_); }

  StringView RangeAt(unsigned start, unsigned length) const {
    DCHECK(start + length <= string_length_);
    return StringView(*string_, start, length);
  }

 private:
  size_t offset_;
  const size_t string_length_;
  const RefPtr<StringImpl> string_;
};

}  // namespace blink

#endif  // CSSTokenizerInputStream_h

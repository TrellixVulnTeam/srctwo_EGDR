/*
    Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "platform/text/SegmentedString.h"

namespace blink {

unsigned SegmentedString::length() const {
  unsigned length = current_string_.length();
  if (IsComposite()) {
    Deque<SegmentedSubstring>::const_iterator it = substrings_.begin();
    Deque<SegmentedSubstring>::const_iterator e = substrings_.end();
    for (; it != e; ++it)
      length += it->length();
  }
  return length;
}

void SegmentedString::SetExcludeLineNumbers() {
  current_string_.SetExcludeLineNumbers();
  if (IsComposite()) {
    Deque<SegmentedSubstring>::iterator it = substrings_.begin();
    Deque<SegmentedSubstring>::iterator e = substrings_.end();
    for (; it != e; ++it)
      it->SetExcludeLineNumbers();
  }
}

void SegmentedString::Clear() {
  current_char_ = 0;
  current_string_.Clear();
  number_of_characters_consumed_prior_to_current_string_ = 0;
  number_of_characters_consumed_prior_to_current_line_ = 0;
  current_line_ = 0;
  substrings_.clear();
  closed_ = false;
  empty_ = true;
  fast_path_flags_ = kNoFastPath;
  advance_func_ = &SegmentedString::AdvanceEmpty;
  advance_and_update_line_number_func_ = &SegmentedString::AdvanceEmpty;
}

void SegmentedString::Append(const SegmentedSubstring& s) {
  DCHECK(!closed_);
  if (!s.length())
    return;

  if (!current_string_.length()) {
    number_of_characters_consumed_prior_to_current_string_ +=
        current_string_.NumberOfCharactersConsumed();
    current_string_ = s;
    UpdateAdvanceFunctionPointers();
  } else {
    substrings_.push_back(s);
  }
  empty_ = false;
}

void SegmentedString::Push(UChar c) {
  DCHECK(c);

  // pushIfPossible attempts to rewind the pointer in the SegmentedSubstring,
  // however it will fail if the SegmentedSubstring is empty, or
  // when we prepended some text while consuming a SegmentedSubstring by
  // document.write().
  if (current_string_.PushIfPossible(c)) {
    current_char_ = c;
    return;
  }

  Prepend(SegmentedString(String(&c, 1)), PrependType::kUnconsume);
}

void SegmentedString::Prepend(const SegmentedSubstring& s, PrependType type) {
  DCHECK(!s.NumberOfCharactersConsumed());
  if (!s.length())
    return;

  // FIXME: We're also ASSERTing that s is a fresh SegmentedSubstring.
  //        The assumption is sufficient for our current use, but we might
  //        need to handle the more elaborate cases in the future.
  number_of_characters_consumed_prior_to_current_string_ +=
      current_string_.NumberOfCharactersConsumed();
  if (type == PrependType::kUnconsume)
    number_of_characters_consumed_prior_to_current_string_ -= s.length();
  if (!current_string_.length()) {
    current_string_ = s;
    UpdateAdvanceFunctionPointers();
  } else {
    // Shift our m_currentString into our list.
    substrings_.push_front(current_string_);
    current_string_ = s;
    UpdateAdvanceFunctionPointers();
  }
  empty_ = false;
}

void SegmentedString::Close() {
  // Closing a stream twice is likely a coding mistake.
  DCHECK(!closed_);
  closed_ = true;
}

void SegmentedString::Append(const SegmentedString& s) {
  DCHECK(!closed_);

  Append(s.current_string_);
  if (s.IsComposite()) {
    Deque<SegmentedSubstring>::const_iterator it = s.substrings_.begin();
    Deque<SegmentedSubstring>::const_iterator e = s.substrings_.end();
    for (; it != e; ++it)
      Append(*it);
  }
  current_char_ =
      current_string_.length() ? current_string_.GetCurrentChar() : 0;
}

void SegmentedString::Prepend(const SegmentedString& s, PrependType type) {
  if (s.IsComposite()) {
    Deque<SegmentedSubstring>::const_reverse_iterator it =
        s.substrings_.rbegin();
    Deque<SegmentedSubstring>::const_reverse_iterator e = s.substrings_.rend();
    for (; it != e; ++it)
      Prepend(*it, type);
  }
  Prepend(s.current_string_, type);
  current_char_ =
      current_string_.length() ? current_string_.GetCurrentChar() : 0;
}

void SegmentedString::AdvanceSubstring() {
  if (IsComposite()) {
    number_of_characters_consumed_prior_to_current_string_ +=
        current_string_.NumberOfCharactersConsumed();
    current_string_ = substrings_.TakeFirst();
    // If we've previously consumed some characters of the non-current
    // string, we now account for those characters as part of the current
    // string, not as part of "prior to current string."
    number_of_characters_consumed_prior_to_current_string_ -=
        current_string_.NumberOfCharactersConsumed();
    UpdateAdvanceFunctionPointers();
  } else {
    current_string_.Clear();
    empty_ = true;
    fast_path_flags_ = kNoFastPath;
    advance_func_ = &SegmentedString::AdvanceEmpty;
    advance_and_update_line_number_func_ = &SegmentedString::AdvanceEmpty;
  }
}

String SegmentedString::ToString() const {
  StringBuilder result;
  current_string_.AppendTo(result);
  if (IsComposite()) {
    Deque<SegmentedSubstring>::const_iterator it = substrings_.begin();
    Deque<SegmentedSubstring>::const_iterator e = substrings_.end();
    for (; it != e; ++it)
      it->AppendTo(result);
  }
  return result.ToString();
}

void SegmentedString::Advance(unsigned count, UChar* consumed_characters) {
  SECURITY_DCHECK(count <= length());
  for (unsigned i = 0; i < count; ++i) {
    consumed_characters[i] = CurrentChar();
    Advance();
  }
}

void SegmentedString::Advance8() {
  DecrementAndCheckLength();
  current_char_ = current_string_.IncrementAndGetCurrentChar8();
}

void SegmentedString::Advance16() {
  DecrementAndCheckLength();
  current_char_ = current_string_.IncrementAndGetCurrentChar16();
}

void SegmentedString::AdvanceAndUpdateLineNumber8() {
  DCHECK_EQ(current_string_.GetCurrentChar(), current_char_);
  if (current_char_ == '\n') {
    ++current_line_;
    number_of_characters_consumed_prior_to_current_line_ =
        NumberOfCharactersConsumed() + 1;
  }
  DecrementAndCheckLength();
  current_char_ = current_string_.IncrementAndGetCurrentChar8();
}

void SegmentedString::AdvanceAndUpdateLineNumber16() {
  DCHECK_EQ(current_string_.GetCurrentChar(), current_char_);
  if (current_char_ == '\n') {
    ++current_line_;
    number_of_characters_consumed_prior_to_current_line_ =
        NumberOfCharactersConsumed() + 1;
  }
  DecrementAndCheckLength();
  current_char_ = current_string_.IncrementAndGetCurrentChar16();
}

void SegmentedString::AdvanceSlowCase() {
  if (current_string_.length()) {
    current_string_.DecrementLength();
    if (!current_string_.length())
      AdvanceSubstring();
  } else if (!IsComposite()) {
    current_string_.Clear();
    empty_ = true;
    fast_path_flags_ = kNoFastPath;
    advance_func_ = &SegmentedString::AdvanceEmpty;
    advance_and_update_line_number_func_ = &SegmentedString::AdvanceEmpty;
  }
  current_char_ =
      current_string_.length() ? current_string_.GetCurrentChar() : 0;
}

void SegmentedString::AdvanceAndUpdateLineNumberSlowCase() {
  if (current_string_.length()) {
    if (current_string_.GetCurrentChar() == '\n' &&
        current_string_.DoNotExcludeLineNumbers()) {
      ++current_line_;
      // Plus 1 because numberOfCharactersConsumed value hasn't incremented yet;
      // it does with length() decrement below.
      number_of_characters_consumed_prior_to_current_line_ =
          NumberOfCharactersConsumed() + 1;
    }
    current_string_.DecrementLength();
    if (!current_string_.length())
      AdvanceSubstring();
    else
      current_string_.IncrementAndGetCurrentChar();  // Only need the ++
  } else if (!IsComposite()) {
    current_string_.Clear();
    empty_ = true;
    fast_path_flags_ = kNoFastPath;
    advance_func_ = &SegmentedString::AdvanceEmpty;
    advance_and_update_line_number_func_ = &SegmentedString::AdvanceEmpty;
  }

  current_char_ =
      current_string_.length() ? current_string_.GetCurrentChar() : 0;
}

void SegmentedString::AdvanceEmpty() {
  DCHECK(!current_string_.length());
  DCHECK(!IsComposite());
  current_char_ = 0;
}

void SegmentedString::UpdateSlowCaseFunctionPointers() {
  fast_path_flags_ = kNoFastPath;
  advance_func_ = &SegmentedString::AdvanceSlowCase;
  advance_and_update_line_number_func_ =
      &SegmentedString::AdvanceAndUpdateLineNumberSlowCase;
}

OrdinalNumber SegmentedString::CurrentLine() const {
  return OrdinalNumber::FromZeroBasedInt(current_line_);
}

OrdinalNumber SegmentedString::CurrentColumn() const {
  int zero_based_column = NumberOfCharactersConsumed() -
                          number_of_characters_consumed_prior_to_current_line_;
  return OrdinalNumber::FromZeroBasedInt(zero_based_column);
}

void SegmentedString::SetCurrentPosition(OrdinalNumber line,
                                         OrdinalNumber column_aftre_prolog,
                                         int prolog_length) {
  current_line_ = line.ZeroBasedInt();
  number_of_characters_consumed_prior_to_current_line_ =
      NumberOfCharactersConsumed() + prolog_length -
      column_aftre_prolog.ZeroBasedInt();
}

}  // namespace blink

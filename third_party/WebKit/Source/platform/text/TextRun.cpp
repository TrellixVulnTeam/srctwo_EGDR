/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "platform/text/TextRun.h"

#include "platform/RuntimeEnabledFeatures.h"
#include "platform/text/Character.h"

namespace blink {

struct ExpectedTextRunSize {
  DISALLOW_NEW();
  const void* pointer;
  int integers[2];
  float float1;
  float float2;
  float float3;
  uint32_t bitfields : 10;
  TabSize tab_size;
};

static_assert(sizeof(TextRun) == sizeof(ExpectedTextRunSize),
              "TextRun should have expected size");

void TextRun::SetText(const String& string) {
  len_ = string.length();
  if (!len_) {
    data_.characters8 = 0;
    is8_bit_ = true;
    return;
  }
  is8_bit_ = string.Is8Bit();
  if (is8_bit_)
    data_.characters8 = string.Characters8();
  else
    data_.characters16 = string.Characters16();
}

std::unique_ptr<UChar[]> TextRun::NormalizedUTF16(
    unsigned* result_length) const {
  const UChar* source;
  String string_for8_bit_run;
  if (Is8Bit()) {
    string_for8_bit_run =
        String::Make16BitFrom8BitSource(Characters8(), length());
    source = string_for8_bit_run.Characters16();
  } else {
    source = Characters16();
  }

  UChar* buffer = new UChar[len_ + 1];
  *result_length = 0;

  bool error = false;
  unsigned position = 0;
  while (position < len_) {
    UChar32 character;
    U16_NEXT(source, position, len_, character);
    // Don't normalize tabs as they are not treated as spaces for word-end.
    if (NormalizeSpace() &&
        Character::IsNormalizedCanvasSpaceCharacter(character)) {
      character = kSpaceCharacter;
    } else if (Character::TreatAsSpace(character) &&
               character != kNoBreakSpaceCharacter) {
      character = kSpaceCharacter;
    } else if (!RuntimeEnabledFeatures::
                   RenderUnicodeControlCharactersEnabled() &&
               Character::LegacyTreatAsZeroWidthSpaceInComplexScript(
                   character)) {
      character = kZeroWidthSpaceCharacter;
    } else if (Character::TreatAsZeroWidthSpaceInComplexScript(character)) {
      character = kZeroWidthSpaceCharacter;
    }

    U16_APPEND(buffer, *result_length, len_, character, error);
    DCHECK(!error);
  }

  DCHECK(*result_length <= len_);
  return WrapArrayUnique(buffer);
}

unsigned TextRun::IndexOfSubRun(const TextRun& sub_run) const {
  if (Is8Bit() == sub_run.Is8Bit() && sub_run.Bytes() >= Bytes()) {
    size_t start_index = Is8Bit() ? sub_run.Characters8() - Characters8()
                                  : sub_run.Characters16() - Characters16();
    if (start_index + sub_run.length() <= length())
      return start_index;
  }
  return std::numeric_limits<unsigned>::max();
}

}  // namespace blink

/*
 * Copyright (C) 2008, 2009 Apple Inc. All rights reserved.
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

#ifndef FontDataForRangeSet_h
#define FontDataForRangeSet_h

#include "platform/fonts/FontData.h"
#include "platform/fonts/SimpleFontData.h"
#include "platform/fonts/UnicodeRangeSet.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/text/CharacterNames.h"

namespace blink {

class SimpleFontData;

class PLATFORM_EXPORT FontDataForRangeSet
    : public RefCounted<FontDataForRangeSet> {
 public:
  explicit FontDataForRangeSet(PassRefPtr<SimpleFontData> font_data = nullptr,
                               PassRefPtr<UnicodeRangeSet> range_set = nullptr)
      : font_data_(std::move(font_data)), range_set_(std::move(range_set)) {}

  FontDataForRangeSet(const FontDataForRangeSet& other);

  virtual ~FontDataForRangeSet(){};

  bool Contains(UChar32 test_char) const {
    return !range_set_ || range_set_->Contains(test_char);
  }
  bool IsEntireRange() const {
    return !range_set_ || range_set_->IsEntireRange();
  }
  UnicodeRangeSet* Ranges() const { return range_set_.Get(); }
  bool HasFontData() const { return font_data_.Get(); }
  const SimpleFontData* FontData() const { return font_data_.Get(); }

 protected:
  RefPtr<SimpleFontData> font_data_;
  RefPtr<UnicodeRangeSet> range_set_;
};

class PLATFORM_EXPORT FontDataForRangeSetFromCache
    : public FontDataForRangeSet {
 public:
  explicit FontDataForRangeSetFromCache(
      PassRefPtr<SimpleFontData> font_data,
      PassRefPtr<UnicodeRangeSet> range_set = nullptr)
      : FontDataForRangeSet(std::move(font_data), std::move(range_set)) {}
  virtual ~FontDataForRangeSetFromCache();
};

}  // namespace blink

#endif

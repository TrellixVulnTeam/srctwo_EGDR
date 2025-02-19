/*
 * Copyright (C) 2007 Apple Computer, Inc.
 * Copyright (c) 2007, 2008, 2009, Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef FontCustomPlatformData_h
#define FontCustomPlatformData_h

#include "platform/PlatformExport.h"
#include "platform/fonts/FontOrientation.h"
#include "platform/fonts/FontSelectionTypes.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/Forward.h"
#include "platform/wtf/Noncopyable.h"
#include "platform/wtf/RefCounted.h"
#include "platform/wtf/text/WTFString.h"
#include "third_party/skia/include/core/SkRefCnt.h"

class SkTypeface;

namespace blink {

class FontPlatformData;
class SharedBuffer;
class FontVariationSettings;

class PLATFORM_EXPORT FontCustomPlatformData
    : public RefCounted<FontCustomPlatformData> {
  USING_FAST_MALLOC(FontCustomPlatformData);
  WTF_MAKE_NONCOPYABLE(FontCustomPlatformData);

 public:
  static PassRefPtr<FontCustomPlatformData> Create(SharedBuffer*,
                                                   String& ots_parse_message);
  ~FontCustomPlatformData();

  FontPlatformData GetFontPlatformData(
      float size,
      bool bold,
      bool italic,
      const FontSelectionRequest&,
      const FontSelectionCapabilities&,
      FontOrientation = FontOrientation::kHorizontal,
      const FontVariationSettings* = nullptr);

  size_t DataSize() const { return data_size_; }
  static bool SupportsFormat(const String&);

 private:
  // These values are written to logs.  New enum values can be added, but
  // existing enums must never be renumbered or deleted and reused.
  enum WebFontInstantiationResult {
    kErrorInstantiatingVariableFont = 0,
    kSuccessConventionalWebFont = 1,
    kSuccessVariableWebFont = 2,
    kMaxWebFontInstantiationResult = 3
  };

  static void ReportWebFontInstantiationResult(WebFontInstantiationResult);
  FontCustomPlatformData(sk_sp<SkTypeface>, size_t data_size);
  sk_sp<SkTypeface> base_typeface_;
  size_t data_size_;
};

}  // namespace blink

#endif  // FontCustomPlatformData_h

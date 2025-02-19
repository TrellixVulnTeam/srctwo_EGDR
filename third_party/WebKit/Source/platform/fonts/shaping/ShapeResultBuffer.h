// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ShapeResultBuffer_h
#define ShapeResultBuffer_h

#include "platform/PlatformExport.h"
#include "platform/fonts/shaping/ShapeResult.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/RefPtr.h"
#include "platform/wtf/Vector.h"

namespace blink {

struct CharacterRange;
class FontDescription;
struct GlyphData;
class ShapeResultBloberizer;
class TextRun;

class PLATFORM_EXPORT ShapeResultBuffer {
  WTF_MAKE_NONCOPYABLE(ShapeResultBuffer);
  STACK_ALLOCATED();

 public:
  ShapeResultBuffer() : has_vertical_offsets_(false) {}

  void AppendResult(PassRefPtr<const ShapeResult> result) {
    has_vertical_offsets_ |= result->HasVerticalOffsets();
    results_.push_back(std::move(result));
  }

  bool HasVerticalOffsets() const { return has_vertical_offsets_; }

  int OffsetForPosition(const TextRun&,
                        float target_x,
                        bool include_partial_glyphs) const;
  CharacterRange GetCharacterRange(TextDirection,
                                   float total_width,
                                   unsigned from,
                                   unsigned to) const;
  Vector<CharacterRange> IndividualCharacterRanges(TextDirection,
                                                   float total_width) const;

  static CharacterRange GetCharacterRange(RefPtr<const ShapeResult>,
                                          TextDirection,
                                          float total_width,
                                          unsigned from,
                                          unsigned to);

  struct RunFontData {
    SimpleFontData* font_data_;
    size_t glyph_count_;
  };

  Vector<RunFontData> GetRunFontData() const;

  GlyphData EmphasisMarkGlyphData(const FontDescription&) const;

 private:
  friend class ShapeResultBloberizer;
  static CharacterRange GetCharacterRangeInternal(
      const Vector<RefPtr<const ShapeResult>, 64>&,
      TextDirection,
      float total_width,
      unsigned from,
      unsigned to);

  static void AddRunInfoRanges(const ShapeResult::RunInfo&,
                               float offset,
                               Vector<CharacterRange>&);

  // Empirically, cases where we get more than 50 ShapeResults are extremely
  // rare.
  Vector<RefPtr<const ShapeResult>, 64> results_;
  bool has_vertical_offsets_;
};

}  // namespace blink

#endif  // ShapeResultBuffer_h

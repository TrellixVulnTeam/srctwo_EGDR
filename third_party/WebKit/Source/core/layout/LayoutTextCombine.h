/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef LayoutTextCombine_h
#define LayoutTextCombine_h

#include "core/layout/LayoutText.h"
#include "platform/fonts/Font.h"

namespace blink {

// LayoutTextCombine uses different coordinate systems for layout and
// inlineTextBox, because it is treated as 1em-box character in vertical flow
// for the layout, while its inline box is in horizontal flow.
class LayoutTextCombine final : public LayoutText {
 public:
  LayoutTextCombine(Node*, RefPtr<StringImpl>);

  void UpdateFont();
  bool IsCombined() const { return is_combined_; }
  float CombinedTextWidth(const Font& font) const {
    return font.GetFontDescription().ComputedSize();
  }
  const Font& OriginalFont() const { return Parent()->Style()->GetFont(); }
  void TransformToInlineCoordinates(GraphicsContext&,
                                    const LayoutRect& box_rect,
                                    bool clip = false) const;
  LayoutUnit InlineWidthForLayout() const;

  const char* GetName() const override { return "LayoutTextCombine"; }

 private:
  bool IsCombineText() const override { return true; }
  float Width(unsigned from,
              unsigned length,
              const Font&,
              LayoutUnit x_position,
              TextDirection,
              HashSet<const SimpleFontData*>* fallback_fonts = nullptr,
              FloatRect* glyph_bounds = nullptr,
              float expansion = 0) const override;
  void StyleDidChange(StyleDifference, const ComputedStyle* old_style) override;
  void SetTextInternal(RefPtr<StringImpl>) override;
  void UpdateIsCombined();

  float combined_text_width_;
  float scale_x_;
  bool is_combined_ : 1;
  bool needs_font_update_ : 1;
};

DEFINE_LAYOUT_OBJECT_TYPE_CASTS(LayoutTextCombine, IsCombineText());

inline LayoutUnit LayoutTextCombine::InlineWidthForLayout() const {
  DCHECK(!needs_font_update_);
  return LayoutUnit::FromFloatCeil(combined_text_width_);
}

}  // namespace blink

#endif  // LayoutTextCombine_h

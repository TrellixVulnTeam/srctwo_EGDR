/*
 * Copyright (C) 2006 Oliver Hunt <ojh16@student.canterbury.ac.nz>
 * Copyright (C) 2006, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Rob Buis <buis@kde.org>
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
 */

#ifndef LayoutSVGInlineText_h
#define LayoutSVGInlineText_h

#include "core/layout/LayoutText.h"
#include "core/layout/svg/SVGCharacterData.h"
#include "core/layout/svg/SVGTextMetrics.h"
#include "platform/wtf/Vector.h"

namespace blink {

class LayoutSVGInlineText final : public LayoutText {
 public:
  LayoutSVGInlineText(Node*, RefPtr<StringImpl>);

  bool CharacterStartsNewTextChunk(int position) const;
  SVGCharacterDataMap& CharacterDataMap() { return character_data_map_; }
  const SVGCharacterDataMap& CharacterDataMap() const {
    return character_data_map_;
  }

  const Vector<SVGTextMetrics>& MetricsList() const { return metrics_; }

  float ScalingFactor() const { return scaling_factor_; }
  const Font& ScaledFont() const { return scaled_font_; }
  void UpdateScaledFont();
  void UpdateMetricsList(bool& last_character_was_white_space);
  static void ComputeNewScaledFontForStyle(const LayoutObject&,
                                           float& scaling_factor,
                                           Font& scaled_font);

  // Preserves floating point precision for the use in DRT. It knows how to
  // round and does a better job than enclosingIntRect.
  FloatRect FloatLinesBoundingBox() const;

  RefPtr<StringImpl> OriginalText() const override;

  const char* GetName() const override { return "LayoutSVGInlineText"; }

 private:
  void SetTextInternal(RefPtr<StringImpl>) override;
  void StyleDidChange(StyleDifference, const ComputedStyle*) override;

  void AddMetricsFromRun(const TextRun&, bool& last_character_was_white_space);

  FloatRect ObjectBoundingBox() const override {
    return FloatLinesBoundingBox();
  }

  bool IsOfType(LayoutObjectType type) const override {
    return type == kLayoutObjectSVG || type == kLayoutObjectSVGInlineText ||
           LayoutText::IsOfType(type);
  }

  PositionWithAffinity PositionForPoint(const LayoutPoint&) override;
  LayoutRect LocalCaretRect(
      InlineBox*,
      int caret_offset,
      LayoutUnit* extra_width_to_end_of_line = nullptr) override;
  LayoutRect LinesBoundingBox() const override;
  InlineTextBox* CreateTextBox(int start, unsigned short length) override;

  LayoutRect AbsoluteVisualRect() const final;
  FloatRect VisualRectInLocalSVGCoordinates() const final;

  float scaling_factor_;
  Font scaled_font_;
  SVGCharacterDataMap character_data_map_;
  Vector<SVGTextMetrics> metrics_;
};

DEFINE_LAYOUT_OBJECT_TYPE_CASTS(LayoutSVGInlineText, IsSVGInlineText());

}  // namespace blink

#endif  // LayoutSVGInlineText_h

/*
 * Copyright (C) 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#ifndef SVGInlineTextBox_h
#define SVGInlineTextBox_h

#include "core/layout/line/InlineTextBox.h"
#include "core/layout/svg/SVGTextLayoutEngine.h"

namespace blink {

class TextMatchMarker;

class SVGInlineTextBox final : public InlineTextBox {
 public:
  SVGInlineTextBox(LineLayoutItem, int start, unsigned short length);

  bool IsSVGInlineTextBox() const override { return true; }

  LayoutUnit VirtualLogicalHeight() const override { return logical_height_; }
  void SetLogicalHeight(LayoutUnit height) { logical_height_ = height; }

  int OffsetForPosition(LayoutUnit x,
                        bool include_partial_glyphs = true) const override;
  LayoutUnit PositionForOffset(int offset) const override;

  void Paint(const PaintInfo&,
             const LayoutPoint&,
             LayoutUnit line_top,
             LayoutUnit line_bottom) const override;
  LayoutRect LocalSelectionRect(int start_position,
                                int end_position,
                                bool = true) const override;

  bool MapStartEndPositionsIntoFragmentCoordinates(const SVGTextFragment&,
                                                   int& start_position,
                                                   int& end_position) const;

  // Calculate the bounding rect of all text fragments.
  LayoutRect CalculateBoundaries() const;

  void ClearTextFragments() { text_fragments_.clear(); }
  Vector<SVGTextFragment>& TextFragments() { return text_fragments_; }
  const Vector<SVGTextFragment>& TextFragments() const {
    return text_fragments_;
  }

  void DirtyLineBoxes() override;

  bool StartsNewTextChunk() const { return starts_new_text_chunk_; }
  void SetStartsNewTextChunk(bool new_text_chunk) {
    starts_new_text_chunk_ = new_text_chunk;
  }

  int OffsetForPositionInFragment(const SVGTextFragment&,
                                  LayoutUnit position,
                                  bool include_partial_glyphs) const;
  FloatRect SelectionRectForTextFragment(const SVGTextFragment&,
                                         int fragment_start_position,
                                         int fragment_end_position,
                                         const ComputedStyle&) const;
  TextRun ConstructTextRun(const ComputedStyle&, const SVGTextFragment&) const;

 private:
  void PaintDocumentMarker(GraphicsContext&,
                           const LayoutPoint&,
                           const DocumentMarker&,
                           const ComputedStyle&,
                           const Font&,
                           bool) const final;
  void PaintTextMatchMarkerForeground(const PaintInfo&,
                                      const LayoutPoint&,
                                      const TextMatchMarker&,
                                      const ComputedStyle&,
                                      const Font&) const final;
  void PaintTextMatchMarkerBackground(const PaintInfo&,
                                      const LayoutPoint&,
                                      const TextMatchMarker&,
                                      const ComputedStyle&,
                                      const Font&) const final;

  bool NodeAtPoint(HitTestResult&,
                   const HitTestLocation& location_in_container,
                   const LayoutPoint& accumulated_offset,
                   LayoutUnit line_top,
                   LayoutUnit line_bottom) override;

  LayoutUnit logical_height_;
  bool starts_new_text_chunk_ : 1;
  Vector<SVGTextFragment> text_fragments_;
};

DEFINE_INLINE_BOX_TYPE_CASTS(SVGInlineTextBox);

}  // namespace blink

#endif  // SVGInlineTextBox_h

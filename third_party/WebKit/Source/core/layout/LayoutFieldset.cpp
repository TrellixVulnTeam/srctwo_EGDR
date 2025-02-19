/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2005, 2006 Apple Computer, Inc.
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

#include "core/layout/LayoutFieldset.h"

#include "core/CSSPropertyNames.h"
#include "core/HTMLNames.h"
#include "core/html/HTMLLegendElement.h"
#include "core/paint/FieldsetPainter.h"

namespace blink {

using namespace HTMLNames;

LayoutFieldset::LayoutFieldset(Element* element) : LayoutBlockFlow(element) {}

void LayoutFieldset::ComputePreferredLogicalWidths() {
  LayoutBlockFlow::ComputePreferredLogicalWidths();
  if (LayoutBox* legend = FindInFlowLegend()) {
    int legend_min_width = legend->MinPreferredLogicalWidth().ToInt();

    Length legend_margin_left = legend->Style()->MarginLeft();
    Length legend_margin_right = legend->Style()->MarginRight();

    if (legend_margin_left.IsFixed())
      legend_min_width += legend_margin_left.Value();

    if (legend_margin_right.IsFixed())
      legend_min_width += legend_margin_right.Value();

    min_preferred_logical_width_ =
        max(min_preferred_logical_width_,
            legend_min_width + BorderAndPaddingWidth());
  }
}

LayoutObject* LayoutFieldset::LayoutSpecialExcludedChild(bool relayout_children,
                                                         SubtreeLayoutScope&) {
  LayoutBox* legend = FindInFlowLegend();
  if (legend) {
    LayoutRect old_legend_frame_rect = legend->FrameRect();

    if (relayout_children)
      legend->SetNeedsLayoutAndFullPaintInvalidation(
          LayoutInvalidationReason::kFieldsetChanged);
    legend->LayoutIfNeeded();

    LayoutUnit logical_left;
    if (Style()->IsLeftToRightDirection()) {
      switch (legend->Style()->GetTextAlign()) {
        case ETextAlign::kCenter:
          logical_left = (LogicalWidth() - LogicalWidthForChild(*legend)) / 2;
          break;
        case ETextAlign::kRight:
          logical_left = LogicalWidth() - BorderEnd() - PaddingEnd() -
                         LogicalWidthForChild(*legend);
          break;
        default:
          logical_left =
              BorderStart() + PaddingStart() + MarginStartForChild(*legend);
          break;
      }
    } else {
      switch (legend->Style()->GetTextAlign()) {
        case ETextAlign::kLeft:
          logical_left = BorderStart() + PaddingStart();
          break;
        case ETextAlign::kCenter: {
          // Make sure that the extra pixel goes to the end side in RTL (since
          // it went to the end side in LTR).
          LayoutUnit centered_width =
              LogicalWidth() - LogicalWidthForChild(*legend);
          logical_left = centered_width - centered_width / 2;
          break;
        }
        default:
          logical_left = LogicalWidth() - BorderStart() - PaddingStart() -
                         MarginStartForChild(*legend) -
                         LogicalWidthForChild(*legend);
          break;
      }
    }

    SetLogicalLeftForChild(*legend, logical_left);

    LayoutUnit fieldset_border_before = LayoutUnit(BorderBefore());
    LayoutUnit legend_logical_height = LogicalHeightForChild(*legend);

    LayoutUnit legend_logical_top;
    LayoutUnit collapsed_legend_extent;
    // FIXME: We need to account for the legend's margin before too.
    if (fieldset_border_before > legend_logical_height) {
      // The <legend> is smaller than the associated fieldset before border
      // so the latter determines positioning of the <legend>. The sizing
      // depends
      // on the legend's margins as we want to still follow the author's cues.
      // Firefox completely ignores the margins in this case which seems wrong.
      legend_logical_top = (fieldset_border_before - legend_logical_height) / 2;
      collapsed_legend_extent = max<LayoutUnit>(
          fieldset_border_before, legend_logical_top + legend_logical_height +
                                      MarginAfterForChild(*legend));
    } else {
      collapsed_legend_extent =
          legend_logical_height + MarginAfterForChild(*legend);
    }

    SetLogicalTopForChild(*legend, legend_logical_top);
    SetLogicalHeight(PaddingBefore() + collapsed_legend_extent);

    if (legend->FrameRect() != old_legend_frame_rect) {
      // We need to invalidate the fieldset border if the legend's frame
      // changed.
      SetShouldDoFullPaintInvalidation();
    }
  }
  return legend;
}

LayoutBox* LayoutFieldset::FindInFlowLegend() const {
  for (LayoutObject* legend = FirstChild(); legend;
       legend = legend->NextSibling()) {
    if (legend->IsFloatingOrOutOfFlowPositioned())
      continue;

    if (isHTMLLegendElement(legend->GetNode()))
      return ToLayoutBox(legend);
  }
  return nullptr;
}

void LayoutFieldset::PaintBoxDecorationBackground(
    const PaintInfo& paint_info,
    const LayoutPoint& paint_offset) const {
  FieldsetPainter(*this).PaintBoxDecorationBackground(paint_info, paint_offset);
}

void LayoutFieldset::PaintMask(const PaintInfo& paint_info,
                               const LayoutPoint& paint_offset) const {
  FieldsetPainter(*this).PaintMask(paint_info, paint_offset);
}

}  // namespace blink

/*
 * Copyright (C) 2006 Oliver Hunt <ojh16@student.canterbury.ac.nz>
 * Copyright (C) 2006 Apple Computer Inc.
 * Copyright (C) 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2011 Torch Mobile (Beijing) CO. Ltd. All rights reserved.
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

#include "core/layout/svg/line/SVGRootInlineBox.h"

#include "core/layout/api/LineLayoutAPIShim.h"
#include "core/layout/api/LineLayoutBlockFlow.h"
#include "core/layout/api/LineLayoutSVGInlineText.h"
#include "core/layout/svg/LayoutSVGText.h"
#include "core/layout/svg/SVGTextLayoutEngine.h"
#include "core/layout/svg/line/SVGInlineFlowBox.h"
#include "core/layout/svg/line/SVGInlineTextBox.h"
#include "core/paint/SVGRootInlineBoxPainter.h"

namespace blink {

void SVGRootInlineBox::Paint(const PaintInfo& paint_info,
                             const LayoutPoint& paint_offset,
                             LayoutUnit,
                             LayoutUnit) const {
  SVGRootInlineBoxPainter(*this).Paint(paint_info, paint_offset);
}

void SVGRootInlineBox::MarkDirty() {
  for (InlineBox* child = FirstChild(); child; child = child->NextOnLine())
    child->MarkDirty();
  RootInlineBox::MarkDirty();
}

void SVGRootInlineBox::ComputePerCharacterLayoutInformation() {
  LayoutSVGText& text_root =
      ToLayoutSVGText(*LineLayoutAPIShim::LayoutObjectFrom(Block()));

  const Vector<LayoutSVGInlineText*>& descendant_text_nodes =
      text_root.DescendantTextNodes();
  if (descendant_text_nodes.IsEmpty())
    return;

  if (text_root.NeedsReordering())
    ReorderValueLists();

  // Perform SVG text layout phase two (see SVGTextLayoutEngine for details).
  SVGTextLayoutEngine character_layout(descendant_text_nodes);
  character_layout.LayoutCharactersInTextBoxes(this);

  // Perform SVG text layout phase three (see SVGTextChunkBuilder for details).
  character_layout.FinishLayout();

  // Perform SVG text layout phase four
  // Position & resize all SVGInlineText/FlowBoxes in the inline box tree,
  // resize the root box as well as the LayoutSVGText parent block.
  LayoutInlineBoxes(*this);

  // Let the HTML block space originate from the local SVG coordinate space.
  LineLayoutBlockFlow parent_block = Block();
  parent_block.SetLocation(LayoutPoint());
  // The width could be any value, but set it so that a line box will mirror
  // within the childRect when its coordinates are converted between physical
  // block direction and flipped block direction, for ease of understanding of
  // flipped coordinates. The height doesn't matter.
  parent_block.SetSize(LayoutSize(X() * 2 + Width(), LayoutUnit()));

  SetLineTopBottomPositions(LogicalTop(), LogicalBottom(), LogicalTop(),
                            LogicalBottom());
}

LayoutRect SVGRootInlineBox::LayoutInlineBoxes(InlineBox& box) {
  LayoutRect rect;
  if (box.IsSVGInlineTextBox()) {
    rect = ToSVGInlineTextBox(box).CalculateBoundaries();
  } else {
    for (InlineBox* child = ToInlineFlowBox(box).FirstChild(); child;
         child = child->NextOnLine())
      rect.Unite(LayoutInlineBoxes(*child));
  }

  box.SetX(rect.X());
  box.SetY(rect.Y());
  box.SetLogicalWidth(box.IsHorizontal() ? rect.Width() : rect.Height());
  LayoutUnit logical_height = box.IsHorizontal() ? rect.Height() : rect.Width();
  if (box.IsSVGInlineTextBox())
    ToSVGInlineTextBox(box).SetLogicalHeight(logical_height);
  else if (box.IsSVGInlineFlowBox())
    ToSVGInlineFlowBox(box).SetLogicalHeight(logical_height);
  else
    ToSVGRootInlineBox(box).SetLogicalHeight(logical_height);

  return rect;
}

InlineBox* SVGRootInlineBox::ClosestLeafChildForPosition(
    const LayoutPoint& point) {
  InlineBox* first_leaf = FirstLeafChild();
  InlineBox* last_leaf = LastLeafChild();
  if (first_leaf == last_leaf)
    return first_leaf;

  // FIXME: Check for vertical text!
  InlineBox* closest_leaf = nullptr;
  for (InlineBox* leaf = first_leaf; leaf; leaf = leaf->NextLeafChild()) {
    if (!leaf->IsSVGInlineTextBox())
      continue;
    if (point.Y() < leaf->Y())
      continue;
    if (point.Y() > leaf->Y() + leaf->VirtualLogicalHeight())
      continue;

    closest_leaf = leaf;
    if (point.X() < leaf->X() + leaf->LogicalWidth())
      return leaf;
  }

  return closest_leaf ? closest_leaf : last_leaf;
}

static inline void SwapPositioningValuesInTextBoxes(
    SVGInlineTextBox* first_text_box,
    SVGInlineTextBox* last_text_box) {
  LineLayoutSVGInlineText first_text_node =
      LineLayoutSVGInlineText(first_text_box->GetLineLayoutItem());
  SVGCharacterDataMap& first_character_data_map =
      first_text_node.CharacterDataMap();
  SVGCharacterDataMap::iterator it_first =
      first_character_data_map.find(first_text_box->Start() + 1);
  if (it_first == first_character_data_map.end())
    return;
  LineLayoutSVGInlineText last_text_node =
      LineLayoutSVGInlineText(last_text_box->GetLineLayoutItem());
  SVGCharacterDataMap& last_character_data_map =
      last_text_node.CharacterDataMap();
  SVGCharacterDataMap::iterator it_last =
      last_character_data_map.find(last_text_box->Start() + 1);
  if (it_last == last_character_data_map.end())
    return;
  // We only want to perform the swap if both inline boxes are absolutely
  // positioned.
  std::swap(it_first->value, it_last->value);
}

static inline void ReverseInlineBoxRangeAndValueListsIfNeeded(
    Vector<InlineBox*>::iterator first,
    Vector<InlineBox*>::iterator last) {
  // This is a copy of std::reverse(first, last). It additionally assures
  // that the metrics map within the layoutObjects belonging to the
  // InlineBoxes are reordered as well.
  while (true) {
    if (first == last || first == --last)
      return;

    if ((*last)->IsSVGInlineTextBox() && (*first)->IsSVGInlineTextBox()) {
      SVGInlineTextBox* first_text_box = ToSVGInlineTextBox(*first);
      SVGInlineTextBox* last_text_box = ToSVGInlineTextBox(*last);

      // Reordering is only necessary for BiDi text that is _absolutely_
      // positioned.
      if (first_text_box->Len() == 1 &&
          first_text_box->Len() == last_text_box->Len())
        SwapPositioningValuesInTextBoxes(first_text_box, last_text_box);
    }

    InlineBox* temp = *first;
    *first = *last;
    *last = temp;
    ++first;
  }
}

void SVGRootInlineBox::ReorderValueLists() {
  Vector<InlineBox*> leaf_boxes_in_logical_order;
  CollectLeafBoxesInLogicalOrder(leaf_boxes_in_logical_order,
                                 ReverseInlineBoxRangeAndValueListsIfNeeded);
}

bool SVGRootInlineBox::NodeAtPoint(HitTestResult& result,
                                   const HitTestLocation& location_in_container,
                                   const LayoutPoint& accumulated_offset,
                                   LayoutUnit line_top,
                                   LayoutUnit line_bottom) {
  for (InlineBox* leaf = FirstLeafChild(); leaf; leaf = leaf->NextLeafChild()) {
    if (!leaf->IsSVGInlineTextBox())
      continue;
    if (leaf->NodeAtPoint(result, location_in_container, accumulated_offset,
                          line_top, line_bottom))
      return true;
  }

  return false;
}

}  // namespace blink

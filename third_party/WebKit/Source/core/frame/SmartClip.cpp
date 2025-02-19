/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
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

#include "core/frame/SmartClip.h"

#include "core/dom/ContainerNode.h"
#include "core/dom/Document.h"
#include "core/dom/NodeTraversal.h"
#include "core/frame/LocalDOMWindow.h"
#include "core/frame/LocalFrameView.h"
#include "core/html/HTMLFrameOwnerElement.h"
#include "core/layout/LayoutObject.h"
#include "core/page/Page.h"
#include "platform/wtf/text/StringBuilder.h"

namespace blink {

static IntRect ConvertToContentCoordinatesWithoutCollapsingToZero(
    const IntRect& rect_in_viewport,
    const LocalFrameView* view) {
  IntRect rect_in_contents = view->ViewportToContents(rect_in_viewport);
  if (rect_in_viewport.Width() > 0 && !rect_in_contents.Width())
    rect_in_contents.SetWidth(1);
  if (rect_in_viewport.Height() > 0 && !rect_in_contents.Height())
    rect_in_contents.SetHeight(1);
  return rect_in_contents;
}

static Node* NodeInsideFrame(Node* node) {
  if (node->IsFrameOwnerElement())
    return ToHTMLFrameOwnerElement(node)->contentDocument();
  return nullptr;
}

IntRect SmartClipData::RectInViewport() const {
  return rect_in_viewport_;
}

const String& SmartClipData::ClipData() const {
  return string_;
}

SmartClip::SmartClip(LocalFrame* frame) : frame_(frame) {}

SmartClipData SmartClip::DataForRect(const IntRect& crop_rect_in_viewport) {
  Node* best_node =
      FindBestOverlappingNode(frame_->GetDocument(), crop_rect_in_viewport);
  if (!best_node)
    return SmartClipData();

  if (Node* node_from_frame = NodeInsideFrame(best_node)) {
    // FIXME: This code only hit-tests a single iframe. It seems like we ought
    // support nested frames.
    if (Node* best_node_in_frame =
            FindBestOverlappingNode(node_from_frame, crop_rect_in_viewport))
      best_node = best_node_in_frame;
  }

  HeapVector<Member<Node>> hit_nodes;
  CollectOverlappingChildNodes(best_node, crop_rect_in_viewport, hit_nodes);

  if (hit_nodes.IsEmpty() || hit_nodes.size() == best_node->CountChildren()) {
    hit_nodes.clear();
    hit_nodes.push_back(best_node);
  }

  // Unite won't work with the empty rect, so we initialize to the first rect.
  IntRect united_rects = hit_nodes[0]->PixelSnappedBoundingBox();
  StringBuilder collected_text;
  for (size_t i = 0; i < hit_nodes.size(); ++i) {
    collected_text.Append(ExtractTextFromNode(hit_nodes[i]));
    united_rects.Unite(hit_nodes[i]->PixelSnappedBoundingBox());
  }

  return SmartClipData(
      best_node,
      frame_->GetDocument()->View()->ContentsToViewport(united_rects),
      collected_text.ToString());
}

float SmartClip::PageScaleFactor() {
  return frame_->GetPage()->PageScaleFactor();
}

// This function is a bit of a mystery. If you understand what it does, please
// consider adding a more descriptive name.
Node* SmartClip::MinNodeContainsNodes(Node* min_node, Node* new_node) {
  if (!new_node)
    return min_node;
  if (!min_node)
    return new_node;

  IntRect min_node_rect = min_node->PixelSnappedBoundingBox();
  IntRect new_node_rect = new_node->PixelSnappedBoundingBox();

  Node* parent_min_node = min_node->parentNode();
  Node* parent_new_node = new_node->parentNode();

  if (min_node_rect.Contains(new_node_rect)) {
    if (parent_min_node && parent_new_node &&
        parent_new_node->parentNode() == parent_min_node)
      return parent_min_node;
    return min_node;
  }

  if (new_node_rect.Contains(min_node_rect)) {
    if (parent_min_node && parent_new_node &&
        parent_min_node->parentNode() == parent_new_node)
      return parent_new_node;
    return new_node;
  }

  // This loop appears to find the nearest ancestor of minNode (in DOM order)
  // that contains the newNodeRect. It's very unclear to me why that's an
  // interesting node to find. Presumably this loop will often just return
  // the documentElement.
  Node* node = min_node;
  while (node) {
    if (node->GetLayoutObject()) {
      IntRect node_rect = node->PixelSnappedBoundingBox();
      if (node_rect.Contains(new_node_rect)) {
        return node;
      }
    }
    node = node->parentNode();
  }

  return nullptr;
}

Node* SmartClip::FindBestOverlappingNode(Node* root_node,
                                         const IntRect& crop_rect_in_viewport) {
  if (!root_node)
    return nullptr;

  IntRect resized_crop_rect =
      ConvertToContentCoordinatesWithoutCollapsingToZero(
          crop_rect_in_viewport, root_node->GetDocument().View());

  Node* node = root_node;
  Node* min_node = nullptr;

  while (node) {
    IntRect node_rect = node->PixelSnappedBoundingBox();

    if (node->IsElementNode() &&
        DeprecatedEqualIgnoringCase(
            ToElement(node)->FastGetAttribute(HTMLNames::aria_hiddenAttr),
            "true")) {
      node = NodeTraversal::NextSkippingChildren(*node, root_node);
      continue;
    }

    LayoutObject* layout_object = node->GetLayoutObject();
    if (layout_object && !node_rect.IsEmpty()) {
      if (layout_object->IsText() || layout_object->IsLayoutImage() ||
          node->IsFrameOwnerElement() ||
          (layout_object->Style()->HasBackgroundImage() &&
           !ShouldSkipBackgroundImage(node))) {
        if (resized_crop_rect.Intersects(node_rect)) {
          min_node = MinNodeContainsNodes(min_node, node);
        } else {
          node = NodeTraversal::NextSkippingChildren(*node, root_node);
          continue;
        }
      }
    }
    node = NodeTraversal::Next(*node, root_node);
  }

  return min_node;
}

// This function appears to heuristically guess whether to include a background
// image in the smart clip. It seems to want to include sprites created from
// CSS background images but to skip actual backgrounds.
bool SmartClip::ShouldSkipBackgroundImage(Node* node) {
  DCHECK(node);
  // Apparently we're only interested in background images on spans and divs.
  if (!isHTMLSpanElement(*node) && !isHTMLDivElement(*node))
    return true;

  // This check actually makes a bit of sense. If you're going to sprite an
  // image out of a CSS background, you're probably going to specify a height
  // or a width. On the other hand, if we've got a legit background image,
  // it's very likely the height or the width will be set to auto.
  LayoutObject* layout_object = node->GetLayoutObject();
  if (layout_object && (layout_object->Style()->LogicalHeight().IsAuto() ||
                        layout_object->Style()->LogicalWidth().IsAuto()))
    return true;

  return false;
}

void SmartClip::CollectOverlappingChildNodes(
    Node* parent_node,
    const IntRect& crop_rect_in_viewport,
    HeapVector<Member<Node>>& hit_nodes) {
  if (!parent_node)
    return;
  IntRect resized_crop_rect =
      ConvertToContentCoordinatesWithoutCollapsingToZero(
          crop_rect_in_viewport, parent_node->GetDocument().View());
  for (Node* child = parent_node->firstChild(); child;
       child = child->nextSibling()) {
    IntRect child_rect = child->PixelSnappedBoundingBox();
    if (resized_crop_rect.Intersects(child_rect))
      hit_nodes.push_back(child);
  }
}

String SmartClip::ExtractTextFromNode(Node* node) {
  // Science has proven that no text nodes are ever positioned at y == -99999.
  int prev_y_pos = -99999;

  StringBuilder result;
  for (Node& current_node : NodeTraversal::InclusiveDescendantsOf(*node)) {
    const ComputedStyle* style = current_node.EnsureComputedStyle();
    if (style && style->UserSelect() == EUserSelect::kNone)
      continue;

    if (Node* node_from_frame = NodeInsideFrame(&current_node))
      result.Append(ExtractTextFromNode(node_from_frame));

    IntRect node_rect = current_node.PixelSnappedBoundingBox();
    if (current_node.GetLayoutObject() && !node_rect.IsEmpty()) {
      if (current_node.IsTextNode()) {
        String node_value = current_node.nodeValue();

        // It's unclear why we blacklist solitary "\n" node values.
        // Maybe we're trying to ignore <br> tags somehow?
        if (node_value == "\n")
          node_value = "";

        if (node_rect.Y() != prev_y_pos) {
          prev_y_pos = node_rect.Y();
          result.Append('\n');
        }

        result.Append(node_value);
      }
    }
  }

  return result.ToString();
}

}  // namespace blink

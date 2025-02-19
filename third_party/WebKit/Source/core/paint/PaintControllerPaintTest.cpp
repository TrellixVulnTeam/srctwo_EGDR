// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/paint/PaintControllerPaintTest.h"

#include "core/editing/FrameCaret.h"
#include "core/editing/FrameSelection.h"
#include "core/layout/LayoutText.h"
#include "core/layout/line/InlineTextBox.h"
#include "core/page/FocusController.h"
#include "core/paint/LayoutObjectDrawingRecorder.h"
#include "core/paint/ObjectPaintProperties.h"
#include "core/paint/PaintLayerPainter.h"
#include "platform/graphics/GraphicsContext.h"
#include "platform/graphics/paint/DrawingDisplayItem.h"

namespace blink {

INSTANTIATE_TEST_CASE_P(All,
                        PaintControllerPaintTestForSlimmingPaintV1AndV2,
                        ::testing::Bool());

INSTANTIATE_TEST_CASE_P(All,
                        PaintControllerPaintTestForSlimmingPaintV2,
                        ::testing::Bool());

TEST_P(PaintControllerPaintTestForSlimmingPaintV1AndV2,
       FullDocumentPaintingWithCaret) {
  SetBodyInnerHTML(
      "<div id='div' contentEditable='true' style='outline:none'>XYZ</div>");
  GetDocument().GetPage()->GetFocusController().SetActive(true);
  GetDocument().GetPage()->GetFocusController().SetFocused(true);
  Element& div = *ToElement(GetDocument().body()->firstChild());
  InlineTextBox& text_inline_box =
      *ToLayoutText(div.firstChild()->GetLayoutObject())->FirstTextBox();

  if (RuntimeEnabledFeatures::SlimmingPaintV2Enabled()) {
    EXPECT_DISPLAY_LIST(
        RootPaintController().GetDisplayItemList(), 2,
        TestDisplayItem(GetLayoutView(), kDocumentBackgroundType),
        TestDisplayItem(text_inline_box, kForegroundType));
  } else {
    EXPECT_DISPLAY_LIST(
        RootPaintController().GetDisplayItemList(), 2,
        TestDisplayItem(GetLayoutView(), kDocumentBackgroundType),
        TestDisplayItem(text_inline_box, kForegroundType));
  }

  div.focus();
  GetDocument().View()->UpdateAllLifecyclePhases();

  if (RuntimeEnabledFeatures::SlimmingPaintV2Enabled()) {
    EXPECT_DISPLAY_LIST(
        RootPaintController().GetDisplayItemList(), 3,
        TestDisplayItem(GetLayoutView(), kDocumentBackgroundType),
        TestDisplayItem(text_inline_box, kForegroundType),
        TestDisplayItem(GetDocument()
                            .GetFrame()
                            ->Selection()
                            .CaretDisplayItemClientForTesting(),
                        DisplayItem::kCaret));  // New!
  } else {
    EXPECT_DISPLAY_LIST(
        RootPaintController().GetDisplayItemList(), 3,
        TestDisplayItem(GetLayoutView(), kDocumentBackgroundType),
        TestDisplayItem(text_inline_box, kForegroundType),
        TestDisplayItem(GetDocument()
                            .GetFrame()
                            ->Selection()
                            .CaretDisplayItemClientForTesting(),
                        DisplayItem::kCaret));  // New!
  }
}

TEST_P(PaintControllerPaintTestForSlimmingPaintV1AndV2, InlineRelayout) {
  SetBodyInnerHTML(
      "<div id='div' style='width:100px; height: 200px'>AAAAAAAAAA "
      "BBBBBBBBBB</div>");
  Element& div = *ToElement(GetDocument().body()->firstChild());
  LayoutBlock& div_block =
      *ToLayoutBlock(GetDocument().body()->firstChild()->GetLayoutObject());
  LayoutText& text = *ToLayoutText(div_block.FirstChild());
  InlineTextBox& first_text_box = *text.FirstTextBox();

  if (RuntimeEnabledFeatures::SlimmingPaintV2Enabled()) {
    EXPECT_DISPLAY_LIST(
        RootPaintController().GetDisplayItemList(), 2,
        TestDisplayItem(GetLayoutView(), kDocumentBackgroundType),
        TestDisplayItem(first_text_box, kForegroundType));
  } else {
    EXPECT_DISPLAY_LIST(
        RootPaintController().GetDisplayItemList(), 2,
        TestDisplayItem(GetLayoutView(), kDocumentBackgroundType),
        TestDisplayItem(first_text_box, kForegroundType));
  }

  div.setAttribute(HTMLNames::styleAttr, "width: 10px; height: 200px");
  GetDocument().View()->UpdateAllLifecyclePhases();

  LayoutText& new_text = *ToLayoutText(div_block.FirstChild());
  InlineTextBox& new_first_text_box = *new_text.FirstTextBox();
  InlineTextBox& second_text_box = *new_text.FirstTextBox()->NextTextBox();

  if (RuntimeEnabledFeatures::SlimmingPaintV2Enabled()) {
    EXPECT_DISPLAY_LIST(
        RootPaintController().GetDisplayItemList(), 3,
        TestDisplayItem(GetLayoutView(), kDocumentBackgroundType),
        TestDisplayItem(new_first_text_box, kForegroundType),
        TestDisplayItem(second_text_box, kForegroundType));
  } else {
    EXPECT_DISPLAY_LIST(
        RootPaintController().GetDisplayItemList(), 3,
        TestDisplayItem(GetLayoutView(), kDocumentBackgroundType),
        TestDisplayItem(new_first_text_box, kForegroundType),
        TestDisplayItem(second_text_box, kForegroundType));
  }
}

TEST_P(PaintControllerPaintTestForSlimmingPaintV2, ChunkIdClientCacheFlag) {
  SetBodyInnerHTML(
      "<div id='div' style='width: 200px; height: 200px; opacity: 0.5'>"
      "  <div style='width: 100px; height: 100px; background-color: "
      "blue'></div>"
      "  <div style='width: 100px; height: 100px; background-color: "
      "blue'></div>"
      "</div>");
  LayoutBlock& div = *ToLayoutBlock(GetLayoutObjectByElementId("div"));
  LayoutObject& sub_div = *div.FirstChild();
  LayoutObject& sub_div2 = *sub_div.NextSibling();
  EXPECT_DISPLAY_LIST(RootPaintController().GetDisplayItemList(), 3,
                      TestDisplayItem(GetLayoutView(), kDocumentBackgroundType),
                      TestDisplayItem(sub_div, kBackgroundType),
                      TestDisplayItem(sub_div2, kBackgroundType));

  // Verify that the background does not scroll.
  const PaintChunk& background_chunk = RootPaintController().PaintChunks()[0];
  auto* transform = background_chunk.properties.property_tree_state.Transform();
  EXPECT_EQ(nullptr, transform->ScrollNode());

  const EffectPaintPropertyNode* effect_node =
      div.FirstFragment()->PaintProperties()->Effect();
  EXPECT_EQ(0.5f, effect_node->Opacity());

  const PaintChunk& chunk = RootPaintController().PaintChunks()[1];
  EXPECT_EQ(*div.Layer(), chunk.id.client);
  EXPECT_EQ(effect_node, chunk.properties.property_tree_state.Effect());

  EXPECT_FALSE(div.Layer()->IsJustCreated());
  // Client used by only paint chunks and non-cachaeable display items but not
  // by any cacheable display items won't be marked as validly cached.
  EXPECT_TRUE(RootPaintController().ClientCacheIsValid(*div.Layer()));
  EXPECT_FALSE(RootPaintController().ClientCacheIsValid(div));
  EXPECT_TRUE(RootPaintController().ClientCacheIsValid(sub_div));
}

TEST_P(PaintControllerPaintTestForSlimmingPaintV2, CompositingNoFold) {
  SetBodyInnerHTML(
      "<div id='div' style='width: 200px; height: 200px; opacity: 0.5'>"
      "  <div style='width: 100px; height: 100px; background-color: "
      "blue'></div>"
      "</div>");
  LayoutBlock& div = *ToLayoutBlock(GetLayoutObjectByElementId("div"));
  LayoutObject& sub_div = *div.FirstChild();

  EXPECT_DISPLAY_LIST(RootPaintController().GetDisplayItemList(), 2,
                      TestDisplayItem(GetLayoutView(), kDocumentBackgroundType),
                      TestDisplayItem(sub_div, kBackgroundType));
}

TEST_P(PaintControllerPaintTestForSlimmingPaintV2, FrameScrollingContents) {
  // TODO(wangxianzhu): Fix cull rect issue when painting layered contents
  // under overflow clip (in this case the LayoutView).
  if (RuntimeEnabledFeatures::RootLayerScrollingEnabled())
    return;

  SetBodyInnerHTML(
      "<style>"
      "  ::-webkit-scrollbar { display: none }"
      "  body { margin: 0; width: 10000px; height: 1000px }"
      "  div { position: absolute; width: 100px; height: 100px;"
      "        background: blue; }"
      "</style>"
      "<div id='div1' style='top: 0; left: 0'></div>"
      "<div id='div2' style='top: 3000px; left: 3000px'></div>"
      "<div id='div3' style='top: 6000px; left: 6000px'></div>"
      "<div id='div4' style='top: 9000px; left: 9000px'></div>");

  auto& div1 = *GetLayoutObjectByElementId("div1");
  auto& div2 = *GetLayoutObjectByElementId("div2");
  auto& div3 = *GetLayoutObjectByElementId("div3");
  auto& div4 = *GetLayoutObjectByElementId("div4");

  // Initial cull rect: (0,0 4800x4600)
  EXPECT_DISPLAY_LIST(RootPaintController().GetDisplayItemList(), 4,
                      TestDisplayItem(GetLayoutView(), kDocumentBackgroundType),
                      TestDisplayItem(GetLayoutView(), kScrollHitTestType),
                      TestDisplayItem(div1, kBackgroundType),
                      TestDisplayItem(div2, kBackgroundType));

  GetDocument().View()->LayoutViewportScrollableArea()->SetScrollOffset(
      ScrollOffset(5000, 5000), kProgrammaticScroll);
  GetDocument().View()->UpdateAllLifecyclePhases();

  // Cull rect after scroll: (1000,1000 8800x8600)
  EXPECT_DISPLAY_LIST(RootPaintController().GetDisplayItemList(), 5,
                      TestDisplayItem(GetLayoutView(), kDocumentBackgroundType),
                      TestDisplayItem(GetLayoutView(), kScrollHitTestType),
                      TestDisplayItem(div2, kBackgroundType),
                      TestDisplayItem(div3, kBackgroundType),
                      TestDisplayItem(div4, kBackgroundType));
}

// TODO(wangxianzhu): Fix cull rect issue when painting layered contents under
// overflow clip and add a test case.
TEST_P(PaintControllerPaintTestForSlimmingPaintV2,
       BlockScrollingNonLayeredContents) {
  SetBodyInnerHTML(
      "<style>"
      "  ::-webkit-scrollbar { display: none }"
      "  body { margin: 0 }"
      "  div { width: 100px; height: 100px; background: blue; }"
      "  container { display: block; width: 200px; height: 200px;"
      "              overflow: scroll }"
      "</style>"
      "<container id='container'>"
      "  <div id='div1'></div>"
      "  <div id='div2' style='margin-top: 2900px; margin-left: 3000px'></div>"
      "  <div id='div3' style='margin-top: 2900px; margin-left: 6000px'></div>"
      "  <div id='div4' style='margin-top: 2900px; margin-left: 9000px'></div>"
      "</container>");

  auto& container = *ToLayoutBlock(GetLayoutObjectByElementId("container"));
  auto& div1 = *GetLayoutObjectByElementId("div1");
  auto& div2 = *GetLayoutObjectByElementId("div2");
  auto& div3 = *GetLayoutObjectByElementId("div3");
  auto& div4 = *GetLayoutObjectByElementId("div4");

  // Initial cull rect: (0,0 4200x4200)
  EXPECT_DISPLAY_LIST(RootPaintController().GetDisplayItemList(), 4,
                      TestDisplayItem(GetLayoutView(), kDocumentBackgroundType),
                      TestDisplayItem(container, kScrollHitTestType),
                      TestDisplayItem(div1, kBackgroundType),
                      TestDisplayItem(div2, kBackgroundType));

  container.GetScrollableArea()->SetScrollOffset(ScrollOffset(5000, 5000),
                                                 kProgrammaticScroll);
  GetDocument().View()->UpdateAllLifecyclePhases();

  // Cull rect after scroll: (1000,1000 8100x8100)
  EXPECT_DISPLAY_LIST(RootPaintController().GetDisplayItemList(), 5,
                      TestDisplayItem(GetLayoutView(), kDocumentBackgroundType),
                      TestDisplayItem(container, kScrollHitTestType),
                      TestDisplayItem(div2, kBackgroundType),
                      TestDisplayItem(div3, kBackgroundType),
                      TestDisplayItem(div4, kBackgroundType));
}

TEST_P(PaintControllerPaintTestForSlimmingPaintV2, ScrollHitTestOrder) {
  SetBodyInnerHTML(
      "<style>"
      "  ::-webkit-scrollbar { display: none }"
      "  body { margin: 0 }"
      "  #container { width: 200px; height: 200px;"
      "              overflow: scroll; background: red; }"
      "  #child { width: 100px; height: 300px; background: green; }"
      "  #forceDocumentScroll { height: 1000px; }"
      "</style>"
      "<div id='container'>"
      "  <div id='child'></div>"
      "</div>"
      "<div id='forceDocumentScroll'/>");

  auto& container = *ToLayoutBlock(GetLayoutObjectByElementId("container"));
  auto& child = *GetLayoutObjectByElementId("child");

  // The container's items should all be after the document's scroll hit test
  // to ensure the container is hit before the document. Similarly, the child's
  // items should all be after the container's scroll hit test.
  EXPECT_DISPLAY_LIST(RootPaintController().GetDisplayItemList(), 5,
                      TestDisplayItem(GetLayoutView(), kDocumentBackgroundType),
                      TestDisplayItem(GetLayoutView(), kScrollHitTestType),
                      TestDisplayItem(container, kBackgroundType),
                      TestDisplayItem(container, kScrollHitTestType),
                      TestDisplayItem(child, kBackgroundType));
}

TEST_P(PaintControllerPaintTestForSlimmingPaintV2,
       NonStackingScrollHitTestOrder) {
  SetBodyInnerHTML(
      "<style>"
      "  ::-webkit-scrollbar { display: none }"
      "  body { margin: 0 }"
      "  #container { width: 200px; height: 200px;"
      "              overflow: scroll; background: blue;"
      "              position: relative; z-index: auto; }"
      "  #child { width: 80px; height: 20px; background: white; }"
      "  #negZChild { width: 60px; height: 300px; background: purple;"
      "               position: absolute; z-index: -1; top: 0; }"
      "  #posZChild { width: 40px; height: 300px; background: yellow;"
      "               position: absolute; z-index: 1; top: 0; }"
      "</style>"
      "<div id='container'>"
      "  <div id='child'></div>"
      "  <div id='negZChild'></div>"
      "  <div id='posZChild'></div>"
      "</div>");

  auto& container = *ToLayoutBlock(GetLayoutObjectByElementId("container"));
  auto& child = *GetLayoutObjectByElementId("child");
  auto& neg_z_child = *GetLayoutObjectByElementId("negZChild");
  auto& pos_z_child = *GetLayoutObjectByElementId("posZChild");

  // Container is not a stacking context because no z-index is auto.
  // Negative z-index descendants are painted before the background and
  // positive z-index descendants are painted after the background. Scroll hit
  // testing should hit positive descendants, the container, and then negative
  // descendants so the ScrollHitTest item should be immediately after the
  // background.
  EXPECT_DISPLAY_LIST(RootPaintController().GetDisplayItemList(), 6,
                      TestDisplayItem(GetLayoutView(), kDocumentBackgroundType),
                      TestDisplayItem(neg_z_child, kBackgroundType),
                      TestDisplayItem(container, kBackgroundType),
                      TestDisplayItem(container, kScrollHitTestType),
                      TestDisplayItem(child, kBackgroundType),
                      TestDisplayItem(pos_z_child, kBackgroundType));
}

TEST_P(PaintControllerPaintTestForSlimmingPaintV2, StackingScrollHitTestOrder) {
  SetBodyInnerHTML(
      "<style>"
      "  ::-webkit-scrollbar { display: none }"
      "  body { margin: 0 }"
      "  #container { width: 200px; height: 200px;"
      "              overflow: scroll; background: blue;"
      "              position: relative; z-index: 0; }"
      "  #child { width: 80px; height: 20px; background: white; }"
      "  #negZChild { width: 60px; height: 300px; background: purple;"
      "               position: absolute; z-index: -1; top: 0; }"
      "  #posZChild { width: 40px; height: 300px; background: yellow;"
      "               position: absolute; z-index: 1; top: 0; }"
      "</style>"
      "<div id='container'>"
      "  <div id='child'></div>"
      "  <div id='negZChild'></div>"
      "  <div id='posZChild'></div>"
      "</div>");

  auto& container = *ToLayoutBlock(GetLayoutObjectByElementId("container"));
  auto& child = *GetLayoutObjectByElementId("child");
  auto& neg_z_child = *GetLayoutObjectByElementId("negZChild");
  auto& pos_z_child = *GetLayoutObjectByElementId("posZChild");

  // Container is a stacking context because z-index is non-auto.
  // Both positive and negative z-index descendants are painted after the
  // background. The scroll hit test should be after the background but before
  // the z-index descendants to ensure hit test order is correct.
  EXPECT_DISPLAY_LIST(RootPaintController().GetDisplayItemList(), 6,
                      TestDisplayItem(GetLayoutView(), kDocumentBackgroundType),
                      TestDisplayItem(container, kBackgroundType),
                      TestDisplayItem(container, kScrollHitTestType),
                      TestDisplayItem(neg_z_child, kBackgroundType),
                      TestDisplayItem(child, kBackgroundType),
                      TestDisplayItem(pos_z_child, kBackgroundType));
}

TEST_P(PaintControllerPaintTestForSlimmingPaintV2,
       NonStackingScrollHitTestOrderWithoutBackground) {
  SetBodyInnerHTML(
      "<style>"
      "  ::-webkit-scrollbar { display: none }"
      "  body { margin: 0 }"
      "  #container { width: 200px; height: 200px;"
      "              overflow: scroll; background: transparent;"
      "              position: relative; z-index: auto; }"
      "  #child { width: 80px; height: 20px; background: white; }"
      "  #negZChild { width: 60px; height: 300px; background: purple;"
      "               position: absolute; z-index: -1; top: 0; }"
      "  #posZChild { width: 40px; height: 300px; background: yellow;"
      "               position: absolute; z-index: 1; top: 0; }"
      "</style>"
      "<div id='container'>"
      "  <div id='child'></div>"
      "  <div id='negZChild'></div>"
      "  <div id='posZChild'></div>"
      "</div>");

  auto& container = *ToLayoutBlock(GetLayoutObjectByElementId("container"));
  auto& child = *GetLayoutObjectByElementId("child");
  auto& neg_z_child = *GetLayoutObjectByElementId("negZChild");
  auto& pos_z_child = *GetLayoutObjectByElementId("posZChild");

  // Even though container does not paint a background, the scroll hit test item
  // should still be between the negative z-index child and the regular child.
  EXPECT_DISPLAY_LIST(RootPaintController().GetDisplayItemList(), 5,
                      TestDisplayItem(GetLayoutView(), kDocumentBackgroundType),
                      TestDisplayItem(neg_z_child, kBackgroundType),
                      TestDisplayItem(container, kScrollHitTestType),
                      TestDisplayItem(child, kBackgroundType),
                      TestDisplayItem(pos_z_child, kBackgroundType));
}

}  // namespace blink

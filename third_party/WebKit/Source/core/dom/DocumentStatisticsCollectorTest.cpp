// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/dom/DocumentStatisticsCollector.h"

#include <memory>
#include "core/dom/Document.h"
#include "core/frame/LocalFrameView.h"
#include "core/html/HTMLHeadElement.h"
#include "core/html/HTMLLinkElement.h"
#include "core/testing/DummyPageHolder.h"
#include "platform/wtf/text/StringBuilder.h"
#include "public/platform/WebDistillability.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

// Saturate the length of a paragraph to save time.
const unsigned kTextContentLengthSaturation = 1000;

// Filter out short P elements. The threshold is set to around 2 English
// sentences.
const unsigned kParagraphLengthThreshold = 140;

class DocumentStatisticsCollectorTest : public ::testing::Test {
 protected:
  void SetUp() override;

  void TearDown() override { ThreadState::Current()->CollectAllGarbage(); }

  Document& GetDocument() const { return dummy_page_holder_->GetDocument(); }

  void SetHtmlInnerHTML(const String&);

 private:
  std::unique_ptr<DummyPageHolder> dummy_page_holder_;
};

void DocumentStatisticsCollectorTest::SetUp() {
  dummy_page_holder_ = DummyPageHolder::Create(IntSize(800, 600));
}

void DocumentStatisticsCollectorTest::SetHtmlInnerHTML(
    const String& html_content) {
  GetDocument().documentElement()->setInnerHTML((html_content));
}

// This test checks open graph articles can be recognized.
TEST_F(DocumentStatisticsCollectorTest, HasOpenGraphArticle) {
  SetHtmlInnerHTML(
      "<head>"
      // Note the case-insensitive matching of the word "article".
      "    <meta property='og:type' content='arTiclE' />"
      "</head>");
  WebDistillabilityFeatures features =
      DocumentStatisticsCollector::CollectStatistics(GetDocument());

  EXPECT_TRUE(features.open_graph);
}

// This test checks non-existence of open graph articles can be recognized.
TEST_F(DocumentStatisticsCollectorTest, NoOpenGraphArticle) {
  SetHtmlInnerHTML(
      "<head>"
      "    <meta property='og:type' content='movie' />"
      "</head>");
  WebDistillabilityFeatures features =
      DocumentStatisticsCollector::CollectStatistics(GetDocument());

  EXPECT_FALSE(features.open_graph);
}

// This test checks element counts are correct.
TEST_F(DocumentStatisticsCollectorTest, CountElements) {
  SetHtmlInnerHTML(
      "<form>"
      "    <input type='text'>"
      "    <input type='password'>"
      "</form>"
      "<pre></pre>"
      "<p><a>    </a></p>"
      "<ul><li><p><a>    </a></p></li></ul>");
  WebDistillabilityFeatures features =
      DocumentStatisticsCollector::CollectStatistics(GetDocument());

  EXPECT_FALSE(features.open_graph);

  EXPECT_EQ(10u, features.element_count);
  EXPECT_EQ(2u, features.anchor_count);
  EXPECT_EQ(1u, features.form_count);
  EXPECT_EQ(1u, features.text_input_count);
  EXPECT_EQ(1u, features.password_input_count);
  EXPECT_EQ(2u, features.p_count);
  EXPECT_EQ(1u, features.pre_count);
}

// This test checks score calculations are correct.
TEST_F(DocumentStatisticsCollectorTest, CountScore) {
  SetHtmlInnerHTML(
      "<p class='menu' id='article'>1</p>"  // textContentLength = 1
      "<ul><li><p>12</p></li></ul>"  // textContentLength = 2, skipped because
                                     // under li
      "<p class='menu'>123</p>"      // textContentLength = 3, skipped because
                                     // unlikelyCandidates
      "<p>"
      "12345678901234567890123456789012345678901234567890"
      "12345678901234567890123456789012345678901234567890"
      "12345678901234567890123456789012345678901234"
      "</p>"                               // textContentLength = 144
      "<p style='display:none'>12345</p>"  // textContentLength = 5, skipped
                                           // because invisible
      "<div style='display:none'><p>123456</p></div>"  // textContentLength = 6,
                                                       // skipped because
                                                       // invisible
      "<div style='visibility:hidden'><p>1234567</p></div>"  // textContentLength
                                                             // = 7, skipped
                                                             // because
                                                             // invisible
      "<p style='opacity:0'>12345678</p>"  // textContentLength = 8, skipped
                                           // because invisible
      "<p><a href='#'>1234 </a>6 <b> 9</b></p>"  // textContentLength = 9
      "<ul><li></li><p>123456789012</p></ul>"    // textContentLength = 12
      );
  WebDistillabilityFeatures features =
      DocumentStatisticsCollector::CollectStatistics(GetDocument());

  EXPECT_DOUBLE_EQ(features.moz_score, sqrt(144 - kParagraphLengthThreshold));
  EXPECT_DOUBLE_EQ(features.moz_score_all_sqrt,
                   1 + sqrt(144) + sqrt(9) + sqrt(12));
  EXPECT_DOUBLE_EQ(features.moz_score_all_linear, 1 + 144 + 9 + 12);
}

// This test checks saturation of score calculations is correct.
TEST_F(DocumentStatisticsCollectorTest, CountScoreSaturation) {
  StringBuilder html;
  for (int i = 0; i < 10; i++) {
    html.Append("<p>");
    for (int j = 0; j < 1000; j++) {
      html.Append("0123456789");
    }
    html.Append("</p>");
  }
  SetHtmlInnerHTML(html.ToString());
  WebDistillabilityFeatures features =
      DocumentStatisticsCollector::CollectStatistics(GetDocument());

  double error = 1e-5;
  EXPECT_NEAR(
      features.moz_score,
      6 * sqrt(kTextContentLengthSaturation - kParagraphLengthThreshold),
      error);
  EXPECT_NEAR(features.moz_score_all_sqrt,
              6 * sqrt(kTextContentLengthSaturation), error);
  EXPECT_NEAR(features.moz_score_all_linear, 6 * kTextContentLengthSaturation,
              error);
}

}  // namespace blink

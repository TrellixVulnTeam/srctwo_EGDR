// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/css/StyleSheetContents.h"

#include "core/css/CSSTestHelper.h"
#include "core/css/parser/CSSParser.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

TEST(StyleSheetContentsTest, InsertMediaRule) {
  CSSParserContext* context = CSSParserContext::Create(kHTMLStandardMode);

  StyleSheetContents* style_sheet = StyleSheetContents::Create(context);
  style_sheet->ParseString("@namespace ns url(test);");
  EXPECT_EQ(1U, style_sheet->RuleCount());

  style_sheet->SetMutable();
  style_sheet->WrapperInsertRule(
      CSSParser::ParseRule(context, style_sheet,
                           "@media all { div { color: pink } }"),
      0);
  EXPECT_EQ(1U, style_sheet->RuleCount());
  EXPECT_TRUE(style_sheet->HasMediaQueries());

  style_sheet->WrapperInsertRule(
      CSSParser::ParseRule(context, style_sheet,
                           "@media all { div { color: green } }"),
      1);
  EXPECT_EQ(2U, style_sheet->RuleCount());
  EXPECT_TRUE(style_sheet->HasMediaQueries());
}

TEST(StyleSheetContentsTest, InsertFontFaceRule) {
  CSSParserContext* context = CSSParserContext::Create(kHTMLStandardMode);

  StyleSheetContents* style_sheet = StyleSheetContents::Create(context);
  style_sheet->ParseString("@namespace ns url(test);");
  EXPECT_EQ(1U, style_sheet->RuleCount());

  style_sheet->SetMutable();
  style_sheet->WrapperInsertRule(
      CSSParser::ParseRule(context, style_sheet,
                           "@font-face { font-family: a }"),
      0);
  EXPECT_EQ(1U, style_sheet->RuleCount());
  EXPECT_TRUE(style_sheet->HasFontFaceRule());

  style_sheet->WrapperInsertRule(
      CSSParser::ParseRule(context, style_sheet,
                           "@font-face { font-family: b }"),
      1);
  EXPECT_EQ(2U, style_sheet->RuleCount());
  EXPECT_TRUE(style_sheet->HasFontFaceRule());
}

TEST(StyleSheetContentsTest, HasViewportRule) {
  CSSParserContext* context = CSSParserContext::Create(kHTMLStandardMode);

  StyleSheetContents* style_sheet = StyleSheetContents::Create(context);
  style_sheet->ParseString("@viewport { width: 200px}");
  EXPECT_EQ(1U, style_sheet->RuleCount());
  EXPECT_TRUE(style_sheet->HasViewportRule());
}

TEST(StyleSheetContentsTest, HasViewportRuleAfterInsertion) {
  CSSParserContext* context = CSSParserContext::Create(kHTMLStandardMode);

  StyleSheetContents* style_sheet = StyleSheetContents::Create(context);
  style_sheet->ParseString("body { color: pink }");
  EXPECT_EQ(1U, style_sheet->RuleCount());
  EXPECT_FALSE(style_sheet->HasViewportRule());

  style_sheet->SetMutable();
  style_sheet->WrapperInsertRule(
      CSSParser::ParseRule(context, style_sheet, "@viewport { width: 200px }"),
      0);
  EXPECT_EQ(2U, style_sheet->RuleCount());
  EXPECT_TRUE(style_sheet->HasViewportRule());
}

TEST(StyleSheetContentsTest, HasViewportRuleAfterInsertionIntoMediaRule) {
  CSSParserContext* context = CSSParserContext::Create(kHTMLStandardMode);

  StyleSheetContents* style_sheet = StyleSheetContents::Create(context);
  style_sheet->ParseString("@media {}");
  ASSERT_EQ(1U, style_sheet->RuleCount());
  EXPECT_FALSE(style_sheet->HasViewportRule());

  StyleRuleMedia* media_rule = ToStyleRuleMedia(style_sheet->RuleAt(0));
  style_sheet->SetMutable();
  media_rule->WrapperInsertRule(
      0,
      CSSParser::ParseRule(context, style_sheet, "@viewport { width: 200px }"));
  EXPECT_EQ(1U, media_rule->ChildRules().size());
  EXPECT_TRUE(style_sheet->HasViewportRule());
}

}  // namespace blink

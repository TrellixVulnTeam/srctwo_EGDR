// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/css/CSSPageRule.h"

#include "core/css/CSSRuleList.h"
#include "core/css/CSSTestHelper.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

TEST(CSSPageRule, Serializing) {
  CSSTestHelper helper;

  const char* css_rule = "@page :left { size: auto; }";
  helper.AddCSSRules(css_rule);
  if (helper.CssRules()) {
    EXPECT_EQ(1u, helper.CssRules()->length());
    EXPECT_EQ(String(css_rule), helper.CssRules()->item(0)->cssText());
    EXPECT_EQ(CSSRule::kPageRule, helper.CssRules()->item(0)->type());
    if (CSSPageRule* page_rule = ToCSSPageRule(helper.CssRules()->item(0)))
      EXPECT_EQ(":left", page_rule->selectorText());
  }
}

TEST(CSSPageRule, selectorText) {
  CSSTestHelper helper;

  const char* css_rule = "@page :left { size: auto; }";
  helper.AddCSSRules(css_rule);
  DCHECK(helper.CssRules());
  EXPECT_EQ(1u, helper.CssRules()->length());

  CSSPageRule* page_rule = ToCSSPageRule(helper.CssRules()->item(0));
  EXPECT_EQ(":left", page_rule->selectorText());

  // set invalid page selector.
  page_rule->setSelectorText(":hover");
  EXPECT_EQ(":left", page_rule->selectorText());

  // set invalid page selector.
  page_rule->setSelectorText("right { bla");
  EXPECT_EQ(":left", page_rule->selectorText());

  // set page pseudo class selector.
  page_rule->setSelectorText(":right");
  EXPECT_EQ(":right", page_rule->selectorText());

  // set page type selector.
  page_rule->setSelectorText("namedpage");
  EXPECT_EQ("namedpage", page_rule->selectorText());
}

}  // namespace blink

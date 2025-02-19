/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * (C) 2002-2003 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2002, 2006, 2008, 2012 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Samuel Weinig (sam@webkit.org)
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

#ifndef CSSGroupingRule_h
#define CSSGroupingRule_h

#include "core/css/CSSRule.h"
#include "core/css/StyleRule.h"
#include "platform/wtf/Vector.h"

namespace blink {

class ExceptionState;
class CSSRuleList;

class CSSGroupingRule : public CSSRule {
  DEFINE_WRAPPERTYPEINFO();

 public:
  ~CSSGroupingRule() override;

  void Reattach(StyleRuleBase*) override;

  CSSRuleList* cssRules() const override;

  unsigned insertRule(const String& rule, unsigned index, ExceptionState&);
  void deleteRule(unsigned index, ExceptionState&);

  // For CSSRuleList
  unsigned length() const;
  CSSRule* Item(unsigned index) const;

  DECLARE_VIRTUAL_TRACE();

 protected:
  CSSGroupingRule(StyleRuleGroup* group_rule, CSSStyleSheet* parent);

  void AppendCSSTextForItems(StringBuilder&) const;

  Member<StyleRuleGroup> group_rule_;
  mutable HeapVector<Member<CSSRule>> child_rule_cssom_wrappers_;
  mutable Member<CSSRuleList> rule_list_cssom_wrapper_;
};

}  // namespace blink

#endif  // CSSGroupingRule_h

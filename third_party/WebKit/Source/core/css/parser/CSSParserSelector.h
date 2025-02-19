/*
 * Copyright (C) 2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2008, 2009, 2010 Apple Inc. All rights
 * reserved.
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

#ifndef CSSParserSelector_h
#define CSSParserSelector_h

#include <memory>
#include "core/CoreExport.h"
#include "core/css/CSSSelector.h"
#include "platform/wtf/PtrUtil.h"

namespace blink {

class CSSParserContext;

class CORE_EXPORT CSSParserSelector {
  WTF_MAKE_NONCOPYABLE(CSSParserSelector);
  USING_FAST_MALLOC(CSSParserSelector);

 public:
  CSSParserSelector();
  explicit CSSParserSelector(const QualifiedName&, bool is_implicit = false);

  static std::unique_ptr<CSSParserSelector> Create() {
    return WTF::WrapUnique(new CSSParserSelector);
  }
  static std::unique_ptr<CSSParserSelector> Create(const QualifiedName& name,
                                                   bool is_implicit = false) {
    return WTF::MakeUnique<CSSParserSelector>(name, is_implicit);
  }

  ~CSSParserSelector();

  std::unique_ptr<CSSSelector> ReleaseSelector() {
    return std::move(selector_);
  }

  CSSSelector::RelationType Relation() const { return selector_->Relation(); }
  void SetValue(const AtomicString& value, bool match_lower_case = false) {
    selector_->SetValue(value, match_lower_case);
  }
  void SetAttribute(const QualifiedName& value,
                    CSSSelector::AttributeMatchType match_type) {
    selector_->SetAttribute(value, match_type);
  }
  void SetArgument(const AtomicString& value) { selector_->SetArgument(value); }
  void SetNth(int a, int b) { selector_->SetNth(a, b); }
  void SetMatch(CSSSelector::MatchType value) { selector_->SetMatch(value); }
  void SetRelation(CSSSelector::RelationType value) {
    selector_->SetRelation(value);
  }
  void SetForPage() { selector_->SetForPage(); }
  void SetRelationIsAffectedByPseudoContent() {
    selector_->SetRelationIsAffectedByPseudoContent();
  }
  bool RelationIsAffectedByPseudoContent() const {
    return selector_->RelationIsAffectedByPseudoContent();
  }

  void UpdatePseudoType(const AtomicString& value,
                        const CSSParserContext& context,
                        bool has_arguments,
                        CSSParserMode mode) const {
    selector_->UpdatePseudoType(value, context, has_arguments, mode);
  }
  void UpdatePseudoPage(const AtomicString& value) {
    selector_->UpdatePseudoPage(value);
  }

  void AdoptSelectorVector(
      Vector<std::unique_ptr<CSSParserSelector>>& selector_vector);
  void SetSelectorList(std::unique_ptr<CSSSelectorList>);

  bool IsHostPseudoSelector() const;

  CSSSelector::MatchType Match() const { return selector_->Match(); }
  CSSSelector::PseudoType GetPseudoType() const {
    return selector_->GetPseudoType();
  }
  const CSSSelectorList* SelectorList() const {
    return selector_->SelectorList();
  }

  bool NeedsImplicitShadowCombinatorForMatching() const {
    return GetPseudoType() == CSSSelector::kPseudoWebKitCustomElement ||
           GetPseudoType() == CSSSelector::kPseudoBlinkInternalElement ||
           GetPseudoType() == CSSSelector::kPseudoCue ||
           GetPseudoType() == CSSSelector::kPseudoPlaceholder ||
           GetPseudoType() == CSSSelector::kPseudoShadow ||
           GetPseudoType() == CSSSelector::kPseudoSlotted;
  }

  bool IsSimple() const;

  CSSParserSelector* TagHistory() const { return tag_history_.get(); }
  void SetTagHistory(std::unique_ptr<CSSParserSelector> selector) {
    tag_history_ = std::move(selector);
  }
  void ClearTagHistory() { tag_history_.reset(); }
  void AppendTagHistory(CSSSelector::RelationType,
                        std::unique_ptr<CSSParserSelector>);
  std::unique_ptr<CSSParserSelector> ReleaseTagHistory();
  void PrependTagSelector(const QualifiedName&, bool tag_is_implicit = false);

 private:
  std::unique_ptr<CSSSelector> selector_;
  std::unique_ptr<CSSParserSelector> tag_history_;
};

}  // namespace blink

#endif

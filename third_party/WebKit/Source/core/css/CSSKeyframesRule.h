/*
 * Copyright (C) 2007, 2008, 2012 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CSSKeyframesRule_h
#define CSSKeyframesRule_h

#include "core/css/CSSRule.h"
#include "core/css/StyleRule.h"
#include "platform/wtf/Forward.h"
#include "platform/wtf/text/AtomicString.h"

namespace blink {

class CSSRuleList;
class CSSKeyframeRule;
class StyleRuleKeyframe;

class StyleRuleKeyframes final : public StyleRuleBase {
 public:
  static StyleRuleKeyframes* Create() { return new StyleRuleKeyframes(); }

  ~StyleRuleKeyframes();

  const HeapVector<Member<StyleRuleKeyframe>>& Keyframes() const {
    return keyframes_;
  }

  void ParserAppendKeyframe(StyleRuleKeyframe*);
  void WrapperAppendKeyframe(StyleRuleKeyframe*);
  void WrapperRemoveKeyframe(unsigned);

  String GetName() const { return name_; }
  void SetName(const String& name) { name_ = AtomicString(name); }

  bool IsVendorPrefixed() const { return is_prefixed_; }
  void SetVendorPrefixed(bool is_prefixed) { is_prefixed_ = is_prefixed; }

  int FindKeyframeIndex(const String& key) const;

  StyleRuleKeyframes* Copy() const { return new StyleRuleKeyframes(*this); }

  DECLARE_TRACE_AFTER_DISPATCH();

  void StyleChanged() { version_++; }
  unsigned Version() const { return version_; }

 private:
  StyleRuleKeyframes();
  explicit StyleRuleKeyframes(const StyleRuleKeyframes&);

  HeapVector<Member<StyleRuleKeyframe>> keyframes_;
  AtomicString name_;
  unsigned version_ : 31;
  unsigned is_prefixed_ : 1;
};

DEFINE_STYLE_RULE_TYPE_CASTS(Keyframes);

class CSSKeyframesRule final : public CSSRule {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static CSSKeyframesRule* Create(StyleRuleKeyframes* rule,
                                  CSSStyleSheet* sheet) {
    return new CSSKeyframesRule(rule, sheet);
  }

  ~CSSKeyframesRule() override;

  StyleRuleKeyframes* Keyframes() { return keyframes_rule_.Get(); }

  String cssText() const override;
  void Reattach(StyleRuleBase*) override;

  String name() const { return keyframes_rule_->GetName(); }
  void setName(const String&);

  CSSRuleList* cssRules() const override;

  void appendRule(const String& rule);
  void deleteRule(const String& key);
  CSSKeyframeRule* findRule(const String& key);

  // For IndexedGetter and CSSRuleList.
  unsigned length() const;
  CSSKeyframeRule* Item(unsigned index) const;
  CSSKeyframeRule* AnonymousIndexedGetter(unsigned index) const;

  bool IsVendorPrefixed() const { return is_prefixed_; }
  void SetVendorPrefixed(bool is_prefixed) { is_prefixed_ = is_prefixed; }

  void StyleChanged() { keyframes_rule_->StyleChanged(); }

  DECLARE_VIRTUAL_TRACE();

 private:
  CSSKeyframesRule(StyleRuleKeyframes*, CSSStyleSheet* parent);

  CSSRule::Type type() const override { return kKeyframesRule; }

  Member<StyleRuleKeyframes> keyframes_rule_;
  mutable HeapVector<Member<CSSKeyframeRule>> child_rule_cssom_wrappers_;
  mutable Member<CSSRuleList> rule_list_cssom_wrapper_;
  bool is_prefixed_;
};

DEFINE_CSS_RULE_TYPE_CASTS(CSSKeyframesRule, kKeyframesRule);

}  // namespace blink

#endif  // CSSKeyframesRule_h

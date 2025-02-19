// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef StyleRuleNamespace_h
#define StyleRuleNamespace_h

#include "core/css/StyleRule.h"

namespace blink {

// This class is never actually stored anywhere currently, but only used for
// the parser to pass to a stylesheet
class StyleRuleNamespace final : public StyleRuleBase {
 public:
  static StyleRuleNamespace* Create(AtomicString prefix, AtomicString uri) {
    return new StyleRuleNamespace(prefix, uri);
  }

  StyleRuleNamespace* Copy() const {
    return new StyleRuleNamespace(prefix_, uri_);
  }

  AtomicString Prefix() const { return prefix_; }
  AtomicString Uri() const { return uri_; }

  DEFINE_INLINE_TRACE_AFTER_DISPATCH() {
    StyleRuleBase::TraceAfterDispatch(visitor);
  }

 private:
  StyleRuleNamespace(AtomicString prefix, AtomicString uri)
      : StyleRuleBase(kNamespace), prefix_(prefix), uri_(uri) {}

  AtomicString prefix_;
  AtomicString uri_;
};

DEFINE_STYLE_RULE_TYPE_CASTS(Namespace);

}  // namespace blink

#endif  // StyleRuleNamespace_h

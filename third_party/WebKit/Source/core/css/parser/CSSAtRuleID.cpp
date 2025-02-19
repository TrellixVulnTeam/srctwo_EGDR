// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/css/parser/CSSAtRuleID.h"

#include "core/css/parser/CSSParserContext.h"
#include "core/frame/UseCounter.h"

namespace blink {

CSSAtRuleID CssAtRuleID(StringView name) {
  if (EqualIgnoringASCIICase(name, "charset"))
    return kCSSAtRuleCharset;
  if (EqualIgnoringASCIICase(name, "font-face"))
    return kCSSAtRuleFontFace;
  if (EqualIgnoringASCIICase(name, "import"))
    return kCSSAtRuleImport;
  if (EqualIgnoringASCIICase(name, "keyframes"))
    return kCSSAtRuleKeyframes;
  if (EqualIgnoringASCIICase(name, "media"))
    return kCSSAtRuleMedia;
  if (EqualIgnoringASCIICase(name, "namespace"))
    return kCSSAtRuleNamespace;
  if (EqualIgnoringASCIICase(name, "page"))
    return kCSSAtRulePage;
  if (EqualIgnoringASCIICase(name, "supports"))
    return kCSSAtRuleSupports;
  if (EqualIgnoringASCIICase(name, "viewport"))
    return kCSSAtRuleViewport;
  if (EqualIgnoringASCIICase(name, "-webkit-keyframes"))
    return kCSSAtRuleWebkitKeyframes;
  if (EqualIgnoringASCIICase(name, "apply"))
    return kCSSAtRuleApply;
  return kCSSAtRuleInvalid;
}

void CountAtRule(const CSSParserContext* context, CSSAtRuleID rule_id) {
  WebFeature feature;

  switch (rule_id) {
    case kCSSAtRuleCharset:
      feature = WebFeature::kCSSAtRuleCharset;
      break;
    case kCSSAtRuleFontFace:
      feature = WebFeature::kCSSAtRuleFontFace;
      break;
    case kCSSAtRuleImport:
      feature = WebFeature::kCSSAtRuleImport;
      break;
    case kCSSAtRuleKeyframes:
      feature = WebFeature::kCSSAtRuleKeyframes;
      break;
    case kCSSAtRuleMedia:
      feature = WebFeature::kCSSAtRuleMedia;
      break;
    case kCSSAtRuleNamespace:
      feature = WebFeature::kCSSAtRuleNamespace;
      break;
    case kCSSAtRulePage:
      feature = WebFeature::kCSSAtRulePage;
      break;
    case kCSSAtRuleSupports:
      feature = WebFeature::kCSSAtRuleSupports;
      break;
    case kCSSAtRuleViewport:
      feature = WebFeature::kCSSAtRuleViewport;
      break;

    case kCSSAtRuleWebkitKeyframes:
      feature = WebFeature::kCSSAtRuleWebkitKeyframes;
      break;

    case kCSSAtRuleApply:
      feature = WebFeature::kCSSAtRuleApply;
      break;

    case kCSSAtRuleInvalid:
    // fallthrough
    default:
      NOTREACHED();
      return;
  }
  context->Count(feature);
}

}  // namespace blink

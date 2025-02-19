// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/css/StyleColor.h"

#include "core/layout/LayoutTheme.h"

namespace blink {

Color StyleColor::ColorFromKeyword(CSSValueID keyword) {
  if (const char* value_name = getValueName(keyword)) {
    if (const NamedColor* named_color =
            FindColor(value_name, strlen(value_name)))
      return Color(named_color->argb_value);
  }
  return LayoutTheme::GetTheme().SystemColor(keyword);
}

bool StyleColor::IsColorKeyword(CSSValueID id) {
  // Named colors and color keywords:
  //
  // <named-color>
  //   'aqua', 'black', 'blue', ..., 'yellow' (CSS3: "basic color keywords")
  //   'aliceblue', ..., 'yellowgreen'        (CSS3: "extended color keywords")
  //   'transparent'
  //
  // 'currentcolor'
  //
  // <deprecated-system-color>
  //   'ActiveBorder', ..., 'WindowText'
  //
  // WebKit proprietary/internal:
  //   '-webkit-link'
  //   '-webkit-activelink'
  //   '-internal-active-list-box-selection'
  //   '-internal-active-list-box-selection-text'
  //   '-internal-inactive-list-box-selection'
  //   '-internal-inactive-list-box-selection-text'
  //   '-webkit-focus-ring-color'
  //   '-internal-quirk-inherit'
  //
  return (id >= CSSValueAqua && id <= CSSValueInternalQuirkInherit) ||
         (id >= CSSValueAliceblue && id <= CSSValueYellowgreen) ||
         id == CSSValueMenu;
}

bool StyleColor::IsSystemColor(CSSValueID id) {
  return (id >= CSSValueActiveborder && id <= CSSValueWindowtext) ||
         id == CSSValueMenu;
}

}  // namespace blink

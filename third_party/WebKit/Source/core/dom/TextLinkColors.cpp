/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 *           (C) 2006 Alexey Proskuryakov (ap@webkit.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012 Apple Inc. All
 * rights reserved.
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved.
 * (http://www.torchmobile.com/)
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) Research In Motion Limited 2010-2011. All rights reserved.
 * Copyright (C) 2013 Google Inc. All rights reserved.
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

#include "core/dom/TextLinkColors.h"

#include "core/css/CSSColorValue.h"
#include "core/css/CSSIdentifierValue.h"
#include "core/css/StyleColor.h"
#include "core/layout/LayoutTheme.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

using namespace cssvalue;

TextLinkColors::TextLinkColors() : text_color_(Color::kBlack) {
  ResetLinkColor();
  ResetVisitedLinkColor();
  ResetActiveLinkColor();
}

void TextLinkColors::ResetLinkColor() {
  link_color_ = Color(0, 0, 238);
}

void TextLinkColors::ResetVisitedLinkColor() {
  visited_link_color_ = Color(85, 26, 139);
}

void TextLinkColors::ResetActiveLinkColor() {
  active_link_color_ = Color(255, 0, 0);
}

Color TextLinkColors::ColorFromCSSValue(const CSSValue& value,
                                        Color current_color,
                                        bool for_visited_link) const {
  if (value.IsColorValue())
    return ToCSSColorValue(value).Value();

  CSSValueID value_id = ToCSSIdentifierValue(value).GetValueID();
  switch (value_id) {
    case CSSValueInvalid:
      NOTREACHED();
      return Color();
    case CSSValueInternalQuirkInherit:
      return TextColor();
    case CSSValueWebkitLink:
      return for_visited_link ? VisitedLinkColor() : LinkColor();
    case CSSValueWebkitActivelink:
      return ActiveLinkColor();
    case CSSValueWebkitFocusRingColor:
      return LayoutTheme::GetTheme().FocusRingColor();
    case CSSValueCurrentcolor:
      return current_color;
    default:
      return StyleColor::ColorFromKeyword(value_id);
  }
}

}  // namespace blink

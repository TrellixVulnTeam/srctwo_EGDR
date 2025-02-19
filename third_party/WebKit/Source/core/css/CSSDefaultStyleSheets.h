/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Apple Inc.
 * All rights reserved.
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
 *
 */

#ifndef CSSDefaultStyleSheets_h
#define CSSDefaultStyleSheets_h

#include "platform/heap/Handle.h"
#include "platform/wtf/Allocator.h"

namespace blink {

class Element;
class RuleSet;
class StyleSheetContents;

class CSSDefaultStyleSheets : public GarbageCollected<CSSDefaultStyleSheets> {
  WTF_MAKE_NONCOPYABLE(CSSDefaultStyleSheets);

 public:
  static CSSDefaultStyleSheets& Instance();

  bool EnsureDefaultStyleSheetsForElement(const Element&);
  void EnsureDefaultStyleSheetForFullscreen();

  RuleSet* DefaultStyle() { return default_style_.Get(); }
  RuleSet* DefaultQuirksStyle() { return default_quirks_style_.Get(); }
  RuleSet* DefaultPrintStyle() { return default_print_style_.Get(); }
  RuleSet* DefaultViewSourceStyle();

  StyleSheetContents* EnsureMobileViewportStyleSheet();
  StyleSheetContents* EnsureTelevisionViewportStyleSheet();
  StyleSheetContents* EnsureXHTMLMobileProfileStyleSheet();

  StyleSheetContents* DefaultStyleSheet() { return default_style_sheet_.Get(); }
  StyleSheetContents* QuirksStyleSheet() { return quirks_style_sheet_.Get(); }
  StyleSheetContents* SvgStyleSheet() { return svg_style_sheet_.Get(); }
  StyleSheetContents* MathmlStyleSheet() { return mathml_style_sheet_.Get(); }
  StyleSheetContents* MediaControlsStyleSheet() {
    return media_controls_style_sheet_.Get();
  }
  StyleSheetContents* FullscreenStyleSheet() {
    return fullscreen_style_sheet_.Get();
  }

  DECLARE_TRACE();

 private:
  CSSDefaultStyleSheets();

  Member<RuleSet> default_style_;
  Member<RuleSet> default_quirks_style_;
  Member<RuleSet> default_print_style_;
  Member<RuleSet> default_view_source_style_;

  Member<StyleSheetContents> default_style_sheet_;
  Member<StyleSheetContents> mobile_viewport_style_sheet_;
  Member<StyleSheetContents> television_viewport_style_sheet_;
  Member<StyleSheetContents> xhtml_mobile_profile_style_sheet_;
  Member<StyleSheetContents> quirks_style_sheet_;
  Member<StyleSheetContents> svg_style_sheet_;
  Member<StyleSheetContents> mathml_style_sheet_;
  Member<StyleSheetContents> media_controls_style_sheet_;
  Member<StyleSheetContents> fullscreen_style_sheet_;
};

}  // namespace blink

#endif  // CSSDefaultStyleSheets_h

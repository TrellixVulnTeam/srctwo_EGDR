/*
 * Copyright (C) 2006, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Nicholas Shanks <webkit@nickshanks.com>
 * Copyright (C) 2013 Google, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef AlternateFontFamily_h
#define AlternateFontFamily_h

#include "build/build_config.h"
#include "platform/FontFamilyNames.h"
#include "platform/fonts/FontDescription.h"
#include "platform/wtf/text/AtomicString.h"

namespace blink {

// We currently do not support bitmap fonts on windows.
// Instead of trying to construct a bitmap font and then going down the fallback
// path map certain common bitmap fonts to their truetype equivalent up front.
inline const AtomicString& AdjustFamilyNameToAvoidUnsupportedFonts(
    const AtomicString& family_name) {
#if defined(OS_WIN)
  // On Windows, 'Courier New' (truetype font) is always present and
  // 'Courier' is a bitmap font. On Mac on the other hand 'Courier' is
  // a truetype font. Thus pages asking for Courier are better of
  // using 'Courier New' on windows.
  if (EqualIgnoringASCIICase(family_name, FontFamilyNames::Courier))
    return FontFamilyNames::Courier_New;

  // Alias 'MS Sans Serif' (bitmap font) -> 'Microsoft Sans Serif'
  // (truetype font).
  if (EqualIgnoringASCIICase(family_name, FontFamilyNames::MS_Sans_Serif))
    return FontFamilyNames::Microsoft_Sans_Serif;

  // Alias 'MS Serif' (bitmap) -> 'Times New Roman' (truetype font).
  // Alias 'Times' -> 'Times New Roman' (truetype font).
  // There's no 'Microsoft Sans Serif-equivalent' for Serif.
  if (EqualIgnoringASCIICase(family_name, FontFamilyNames::MS_Serif) ||
      EqualIgnoringASCIICase(family_name, FontFamilyNames::Times))
    return FontFamilyNames::Times_New_Roman;
#endif

  return family_name;
}

inline const AtomicString& AlternateFamilyName(
    const AtomicString& family_name) {
  // Alias Courier <-> Courier New
  if (EqualIgnoringASCIICase(family_name, FontFamilyNames::Courier))
    return FontFamilyNames::Courier_New;
#if !defined(OS_WIN)
  // On Windows, Courier New (truetype font) is always present and
  // Courier is a bitmap font. So, we don't want to map Courier New to
  // Courier.
  if (EqualIgnoringASCIICase(family_name, FontFamilyNames::Courier_New))
    return FontFamilyNames::Courier;
#endif

  // Alias Times and Times New Roman.
  if (EqualIgnoringASCIICase(family_name, FontFamilyNames::Times))
    return FontFamilyNames::Times_New_Roman;
  if (EqualIgnoringASCIICase(family_name, FontFamilyNames::Times_New_Roman))
    return FontFamilyNames::Times;

  // Alias Arial and Helvetica
  if (EqualIgnoringASCIICase(family_name, FontFamilyNames::Arial))
    return FontFamilyNames::Helvetica;
  if (EqualIgnoringASCIICase(family_name, FontFamilyNames::Helvetica))
    return FontFamilyNames::Arial;

  return g_empty_atom;
}

inline const AtomicString& GetFallbackFontFamily(
    const FontDescription& description) {
  switch (description.GenericFamily()) {
    case FontDescription::kSansSerifFamily:
      return FontFamilyNames::sans_serif;
    case FontDescription::kSerifFamily:
      return FontFamilyNames::serif;
    case FontDescription::kMonospaceFamily:
      return FontFamilyNames::monospace;
    case FontDescription::kCursiveFamily:
      return FontFamilyNames::cursive;
    case FontDescription::kFantasyFamily:
      return FontFamilyNames::fantasy;
    default:
      // Let the caller use the system default font.
      return g_empty_atom;
  }
}

}  // namespace blink

#endif  // AlternateFontFamily_h

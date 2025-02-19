/*
 * Copyright (C) 2007, 2008, 2011 Apple Inc. All rights reserved.
 * Copyright (C) 2013 Google Inc. All rights reserved.
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

#ifndef FontFaceCache_h
#define FontFaceCache_h

#include "core/CoreExport.h"
#include "core/css/CSSSegmentedFontFace.h"
#include "core/css/FontFace.h"
#include "core/css/StyleRule.h"
#include "platform/fonts/FontSelectionTypes.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/Forward.h"
#include "platform/wtf/HashMap.h"
#include "platform/wtf/ListHashSet.h"
#include "platform/wtf/text/StringHash.h"

namespace blink {

class FontDescription;

class CORE_EXPORT FontFaceCache final {
  DISALLOW_NEW();

 public:
  FontFaceCache();

  void Add(const StyleRuleFontFace*, FontFace*);
  void Remove(const StyleRuleFontFace*);
  void ClearCSSConnected();
  void ClearAll();
  void AddFontFace(FontFace*, bool css_connected);
  void RemoveFontFace(FontFace*, bool css_connected);

  size_t GetNumSegmentedFacesForTesting();

  // FIXME: It's sort of weird that add/remove uses StyleRuleFontFace* as key,
  // but this function uses FontDescription/family pair.
  CSSSegmentedFontFace* Get(const FontDescription&, const AtomicString& family);

  const HeapListHashSet<Member<FontFace>>& CssConnectedFontFaces() const {
    return css_connected_font_faces_;
  }

  unsigned Version() const { return version_; }
  void IncrementVersion();

  DECLARE_TRACE();

 private:
  // Two lookup accelerating cashes are needed: For the font selection
  // algorithm to work and not perform font fallback across
  // FontSelectionCapabilities, we need to bin the incoming @font-faces by same
  // FontSelectionCapabilities, then run the font selection algorithm only by
  // looking at the capabilities of each group. Group here means: Font faces of
  // identicaly capabilities, but differing unicode-range.
  //
  // A second lookup table caches the previously received FontSelectionRequest
  // queries, which is: HeapHashMap <String, HeapHashMap<FontSelectionRequest,
  // CSSSegmentedFontFace>>
  using CapabilitiesSet =
      HeapHashMap<FontSelectionCapabilities, Member<CSSSegmentedFontFace>>;
  using SegmentedFacesByFamily =
      HeapHashMap<String, Member<CapabilitiesSet>, CaseFoldingHash>;
  using FontSelectionQueryResult =
      HeapHashMap<FontSelectionRequestKey,
                  Member<CSSSegmentedFontFace>,
                  FontSelectionRequestKeyHash,
                  WTF::SimpleClassHashTraits<FontSelectionRequestKey>>;
  using FontSelectionQueryCache =
      HeapHashMap<String, Member<FontSelectionQueryResult>, CaseFoldingHash>;

  // All incoming faces added from JS or CSS, bucketed per family.
  SegmentedFacesByFamily segmented_faces_;
  // Previously determined font matching query results, bucketed per family and
  // FontSelectionRequest. A family bucket of this cache gets invalidated when a
  // new face of the same family is added or removed.
  FontSelectionQueryCache font_selection_query_cache_;

  // Used for removing font faces from the segmented_faces_ list when a CSS rule
  // is removed.
  using StyleRuleToFontFace =
      HeapHashMap<Member<const StyleRuleFontFace>, Member<FontFace>>;
  StyleRuleToFontFace style_rule_to_font_face_;

  // Needed for incoming ClearCSSConnected() requests coming in from
  // StyleEngine, which clears all those faces from the FontCache which are
  // originating from CSS, as opposed to those originating from JS.
  HeapListHashSet<Member<FontFace>> css_connected_font_faces_;

  // FIXME: See if this could be ditched
  // Used to compare Font instances, and the usage seems suspect.
  unsigned version_;
};

}  // namespace blink

#endif

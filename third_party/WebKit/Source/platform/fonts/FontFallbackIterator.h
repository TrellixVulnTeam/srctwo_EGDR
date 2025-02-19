// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FontFallbackIterator_h
#define FontFallbackIterator_h

#include "platform/fonts/FontDataForRangeSet.h"
#include "platform/fonts/FontFallbackPriority.h"
#include "platform/wtf/HashMap.h"
#include "platform/wtf/PassRefPtr.h"
#include "platform/wtf/RefCounted.h"
#include "platform/wtf/RefPtr.h"
#include "platform/wtf/Vector.h"
#include "platform/wtf/text/Unicode.h"

namespace blink {

using namespace WTF;

class FontDescription;
class FontFallbackList;
class SimpleFontData;

class FontFallbackIterator : public RefCounted<FontFallbackIterator> {
  WTF_MAKE_NONCOPYABLE(FontFallbackIterator);

 public:
  static PassRefPtr<FontFallbackIterator> Create(const FontDescription&,
                                                 PassRefPtr<FontFallbackList>,
                                                 FontFallbackPriority);

  bool HasNext() const { return fallback_stage_ != kOutOfLuck; };

  // Some system fallback APIs (Windows, Android) require a character, or a
  // portion of the string to be passed.  On Mac and Linux, we get a list of
  // fonts without passing in characters.
  PassRefPtr<FontDataForRangeSet> Next(const Vector<UChar32>& hint_list);

 private:
  FontFallbackIterator(const FontDescription&,
                       PassRefPtr<FontFallbackList>,
                       FontFallbackPriority);
  bool RangeSetContributesForHint(const Vector<UChar32> hint_list,
                                  const FontDataForRangeSet*);
  bool AlreadyLoadingRangeForHintChar(UChar32 hint_char);
  void WillUseRange(const AtomicString& family, const FontDataForRangeSet&);

  PassRefPtr<FontDataForRangeSet> UniqueOrNext(
      PassRefPtr<FontDataForRangeSet> candidate,
      const Vector<UChar32>& hint_list);

  PassRefPtr<SimpleFontData> FallbackPriorityFont(UChar32 hint);
  PassRefPtr<SimpleFontData> UniqueSystemFontForHintList(
      const Vector<UChar32>& hint_list);

  const FontDescription& font_description_;
  RefPtr<FontFallbackList> font_fallback_list_;
  int current_font_data_index_;
  unsigned segmented_face_index_;

  enum FallbackStage {
    kFallbackPriorityFonts,
    kFontGroupFonts,
    kSegmentedFace,
    kPreferencesFonts,
    kSystemFonts,
    kOutOfLuck
  };

  FallbackStage fallback_stage_;
  HashSet<UChar32> previously_asked_for_hint_;
  // FontFallbackIterator is meant for single use by HarfBuzzShaper,
  // traversing through the fonts for shaping only once. We must not return
  // duplicate FontDataForRangeSet objects from the next() iteration functions
  // as returning a duplicate value causes a shaping run that won't return any
  // results.
  HashSet<uint32_t> unique_font_data_for_range_sets_returned_;
  Vector<RefPtr<FontDataForRangeSet>> tracked_loading_range_sets_;
  FontFallbackPriority font_fallback_priority_;
};

}  // namespace blink

#endif

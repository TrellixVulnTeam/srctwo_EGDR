/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
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

#include "platform/fonts/FontFallbackList.h"

#include "platform/FontFamilyNames.h"
#include "platform/fonts/AlternateFontFamily.h"
#include "platform/fonts/FontCache.h"
#include "platform/fonts/FontCacheKey.h"
#include "platform/fonts/FontDescription.h"
#include "platform/fonts/FontFamily.h"
#include "platform/fonts/SegmentedFontData.h"
#include "platform/wtf/text/CharacterNames.h"

namespace blink {

FontFallbackList::FontFallbackList()
    : cached_primary_simple_font_data_(0),
      font_selector_(nullptr),
      font_selector_version_(0),
      family_index_(0),
      generation_(FontCache::GetFontCache()->Generation()),
      has_loading_fallback_(false) {}

void FontFallbackList::Invalidate(FontSelector* font_selector) {
  ReleaseFontData();
  font_list_.clear();
  cached_primary_simple_font_data_ = 0;
  family_index_ = 0;
  has_loading_fallback_ = false;
  if (font_selector_ != font_selector)
    font_selector_ = font_selector;
  font_selector_version_ = font_selector_ ? font_selector_->Version() : 0;
  generation_ = FontCache::GetFontCache()->Generation();
}

void FontFallbackList::ReleaseFontData() {
  unsigned num_fonts = font_list_.size();
  for (unsigned i = 0; i < num_fonts; ++i) {
    if (!font_list_[i]->IsCustomFont()) {
      DCHECK(!font_list_[i]->IsSegmented());
      FontCache::GetFontCache()->ReleaseFontData(
          ToSimpleFontData(font_list_[i]));
    }
  }
  shape_cache_.reset();  // Clear the weak pointer to the cache instance.
}

bool FontFallbackList::LoadingCustomFonts() const {
  if (!has_loading_fallback_)
    return false;

  unsigned num_fonts = font_list_.size();
  for (unsigned i = 0; i < num_fonts; ++i) {
    if (font_list_[i]->IsLoading())
      return true;
  }
  return false;
}

bool FontFallbackList::ShouldSkipDrawing() const {
  if (!has_loading_fallback_)
    return false;

  unsigned num_fonts = font_list_.size();
  for (unsigned i = 0; i < num_fonts; ++i) {
    if (font_list_[i]->ShouldSkipDrawing())
      return true;
  }
  return false;
}

const SimpleFontData* FontFallbackList::DeterminePrimarySimpleFontData(
    const FontDescription& font_description) const {
  bool should_load_custom_font = true;

  for (unsigned font_index = 0;; ++font_index) {
    const FontData* font_data = FontDataAt(font_description, font_index);
    if (!font_data) {
      // All fonts are custom fonts and are loading. Return the first FontData.
      font_data = FontDataAt(font_description, 0);
      if (font_data)
        return font_data->FontDataForCharacter(kSpaceCharacter);

      SimpleFontData* last_resort_fallback =
          FontCache::GetFontCache()
              ->GetLastResortFallbackFont(font_description)
              .Get();
      DCHECK(last_resort_fallback);
      return last_resort_fallback;
    }

    if (font_data->IsSegmented() &&
        !ToSegmentedFontData(font_data)->ContainsCharacter(kSpaceCharacter))
      continue;

    const SimpleFontData* font_data_for_space =
        font_data->FontDataForCharacter(kSpaceCharacter);
    DCHECK(font_data_for_space);

    // When a custom font is loading, we should use the correct fallback font to
    // layout the text.  Here skip the temporary font for the loading custom
    // font which may not act as the correct fallback font.
    if (!font_data_for_space->IsLoadingFallback())
      return font_data_for_space;

    if (font_data->IsSegmented()) {
      const SegmentedFontData* segmented = ToSegmentedFontData(font_data);
      for (unsigned i = 0; i < segmented->NumFaces(); i++) {
        const SimpleFontData* range_font_data =
            segmented->FaceAt(i)->FontData();
        if (!range_font_data->IsLoadingFallback())
          return range_font_data;
      }
      if (font_data->IsLoading())
        should_load_custom_font = false;
    }

    // Begin to load the first custom font if needed.
    if (should_load_custom_font) {
      should_load_custom_font = false;
      font_data_for_space->GetCustomFontData()->BeginLoadIfNeeded();
    }
  }
}

PassRefPtr<FontData> FontFallbackList::GetFontData(
    const FontDescription& font_description,
    int& family_index) const {
  const FontFamily* curr_family = &font_description.Family();
  for (int i = 0; curr_family && i < family_index; i++)
    curr_family = curr_family->Next();

  for (; curr_family; curr_family = curr_family->Next()) {
    family_index++;
    if (curr_family->Family().length()) {
      RefPtr<FontData> result;
      if (font_selector_)
        result = font_selector_->GetFontData(font_description,
                                             curr_family->Family());
      if (!result)
        result = FontCache::GetFontCache()->GetFontData(font_description,
                                                        curr_family->Family());
      if (result)
        return result;
    }
  }
  family_index = kCAllFamiliesScanned;

  if (font_selector_) {
    // Try the user's preferred standard font.
    if (RefPtr<FontData> data = font_selector_->GetFontData(
            font_description, FontFamilyNames::webkit_standard))
      return data;
  }

  // Still no result. Hand back our last resort fallback font.
  return FontCache::GetFontCache()->GetLastResortFallbackFont(font_description);
}

FallbackListCompositeKey FontFallbackList::CompositeKey(
    const FontDescription& font_description) const {
  FallbackListCompositeKey key(font_description);
  const FontFamily* current_family = &font_description.Family();
  while (current_family) {
    if (current_family->Family().length()) {
      FontFaceCreationParams params(
          AdjustFamilyNameToAvoidUnsupportedFonts(current_family->Family()));
      RefPtr<FontData> result;
      if (font_selector_)
        result = font_selector_->GetFontData(font_description,
                                             current_family->Family());
      if (!result) {
        if (FontPlatformData* platform_data =
                FontCache::GetFontCache()->GetFontPlatformData(font_description,
                                                               params))
          result = FontCache::GetFontCache()->FontDataFromFontPlatformData(
              platform_data);
      }
      if (result) {
        key.Add(font_description.CacheKey(params));
        if (!result->IsSegmented() && !result->IsCustomFont())
          FontCache::GetFontCache()->ReleaseFontData(ToSimpleFontData(result));
      }
    }
    current_family = current_family->Next();
  }

  return key;
}

const FontData* FontFallbackList::FontDataAt(
    const FontDescription& font_description,
    unsigned realized_font_index) const {
  if (realized_font_index < font_list_.size())
    return font_list_[realized_font_index]
        .Get();  // This fallback font is already in our list.

  // Make sure we're not passing in some crazy value here.
  DCHECK_EQ(realized_font_index, font_list_.size());

  if (family_index_ == kCAllFamiliesScanned)
    return 0;

  // Ask the font cache for the font data.
  // We are obtaining this font for the first time.  We keep track of the
  // families we've looked at before in |m_familyIndex|, so that we never scan
  // the same spot in the list twice.  getFontData will adjust our
  // |m_familyIndex| as it scans for the right font to make.
  DCHECK_EQ(FontCache::GetFontCache()->Generation(), generation_);
  RefPtr<FontData> result = GetFontData(font_description, family_index_);
  if (result) {
    font_list_.push_back(result);
    if (result->IsLoadingFallback())
      has_loading_fallback_ = true;
  }
  return result.Get();
}

bool FontFallbackList::IsValid() const {
  if (!font_selector_)
    return font_selector_version_ == 0;

  return font_selector_->Version() == font_selector_version_;
}

}  // namespace blink

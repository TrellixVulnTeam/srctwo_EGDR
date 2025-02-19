/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "platform/fonts/GenericFontFamilySettings.h"

#include "platform/fonts/FontCache.h"

namespace blink {

GenericFontFamilySettings::GenericFontFamilySettings(
    const GenericFontFamilySettings& other)
    : standard_font_family_map_(other.standard_font_family_map_),
      serif_font_family_map_(other.serif_font_family_map_),
      fixed_font_family_map_(other.fixed_font_family_map_),
      sans_serif_font_family_map_(other.sans_serif_font_family_map_),
      cursive_font_family_map_(other.cursive_font_family_map_),
      fantasy_font_family_map_(other.fantasy_font_family_map_),
      pictograph_font_family_map_(other.pictograph_font_family_map_) {}

void GenericFontFamilySettings::IsolatedCopyTo(
    GenericFontFamilySettings& dest) const {
  DCHECK(!IsIsolated());
  auto copy_to_vector = [](const ScriptFontFamilyMap& map,
                           IsolatedCopyVector& vector) {
    for (const auto& kv : map)
      vector.emplace_back(kv.key, kv.value.GetString().IsolatedCopy());
  };

  dest.isolated_copy_ = WTF::MakeUnique<IsolatedCopyVector[]>(7);
  copy_to_vector(standard_font_family_map_, dest.isolated_copy_[0]);
  copy_to_vector(serif_font_family_map_, dest.isolated_copy_[1]);
  copy_to_vector(fixed_font_family_map_, dest.isolated_copy_[2]);
  copy_to_vector(sans_serif_font_family_map_, dest.isolated_copy_[3]);
  copy_to_vector(cursive_font_family_map_, dest.isolated_copy_[4]);
  copy_to_vector(fantasy_font_family_map_, dest.isolated_copy_[5]);
  copy_to_vector(pictograph_font_family_map_, dest.isolated_copy_[6]);
}

void GenericFontFamilySettings::MakeAtomic() {
  DCHECK(IsIsolated());
  auto copy_from_vector = [](ScriptFontFamilyMap& map,
                             const IsolatedCopyVector& vector) {
    for (const auto& kv : vector)
      map.insert(kv.first, AtomicString(kv.second));
  };
  copy_from_vector(standard_font_family_map_, isolated_copy_[0]);
  copy_from_vector(serif_font_family_map_, isolated_copy_[1]);
  copy_from_vector(fixed_font_family_map_, isolated_copy_[2]);
  copy_from_vector(sans_serif_font_family_map_, isolated_copy_[3]);
  copy_from_vector(cursive_font_family_map_, isolated_copy_[4]);
  copy_from_vector(fantasy_font_family_map_, isolated_copy_[5]);
  copy_from_vector(pictograph_font_family_map_, isolated_copy_[6]);
  isolated_copy_.reset();
}

GenericFontFamilySettings& GenericFontFamilySettings::operator=(
    const GenericFontFamilySettings& other) {
  standard_font_family_map_ = other.standard_font_family_map_;
  serif_font_family_map_ = other.serif_font_family_map_;
  fixed_font_family_map_ = other.fixed_font_family_map_;
  sans_serif_font_family_map_ = other.sans_serif_font_family_map_;
  cursive_font_family_map_ = other.cursive_font_family_map_;
  fantasy_font_family_map_ = other.fantasy_font_family_map_;
  pictograph_font_family_map_ = other.pictograph_font_family_map_;
  return *this;
}

// Sets the entry in the font map for the given script. If family is the empty
// string, removes the entry instead.
void GenericFontFamilySettings::SetGenericFontFamilyMap(
    ScriptFontFamilyMap& font_map,
    const AtomicString& family,
    UScriptCode script) {
  ScriptFontFamilyMap::iterator it = font_map.find(static_cast<int>(script));
  if (family.IsEmpty()) {
    if (it == font_map.end())
      return;
    font_map.erase(it);
  } else if (it != font_map.end() && it->value == family) {
    return;
  } else {
    font_map.Set(static_cast<int>(script), family);
  }
}

const AtomicString& GenericFontFamilySettings::GenericFontFamilyForScript(
    const ScriptFontFamilyMap& font_map,
    UScriptCode script) const {
  ScriptFontFamilyMap::iterator it =
      const_cast<ScriptFontFamilyMap&>(font_map).find(static_cast<int>(script));
  if (it != font_map.end()) {
    // Replace with the first available font if it starts with ",".
    if (!it->value.IsEmpty() && it->value[0] == ',')
      it->value = AtomicString(FontCache::FirstAvailableOrFirst(it->value));
    return it->value;
  }
  if (script != USCRIPT_COMMON)
    return GenericFontFamilyForScript(font_map, USCRIPT_COMMON);
  return g_empty_atom;
}

const AtomicString& GenericFontFamilySettings::Standard(
    UScriptCode script) const {
  return GenericFontFamilyForScript(standard_font_family_map_, script);
}

bool GenericFontFamilySettings::UpdateStandard(const AtomicString& family,
                                               UScriptCode script) {
  if (family == Standard())
    return false;
  SetGenericFontFamilyMap(standard_font_family_map_, family, script);
  return true;
}

const AtomicString& GenericFontFamilySettings::Fixed(UScriptCode script) const {
  return GenericFontFamilyForScript(fixed_font_family_map_, script);
}

bool GenericFontFamilySettings::UpdateFixed(const AtomicString& family,
                                            UScriptCode script) {
  if (family == Fixed())
    return false;
  SetGenericFontFamilyMap(fixed_font_family_map_, family, script);
  return true;
}

const AtomicString& GenericFontFamilySettings::Serif(UScriptCode script) const {
  return GenericFontFamilyForScript(serif_font_family_map_, script);
}

bool GenericFontFamilySettings::UpdateSerif(const AtomicString& family,
                                            UScriptCode script) {
  if (family == Serif())
    return false;
  SetGenericFontFamilyMap(serif_font_family_map_, family, script);
  return true;
}

const AtomicString& GenericFontFamilySettings::SansSerif(
    UScriptCode script) const {
  return GenericFontFamilyForScript(sans_serif_font_family_map_, script);
}

bool GenericFontFamilySettings::UpdateSansSerif(const AtomicString& family,
                                                UScriptCode script) {
  if (family == SansSerif())
    return false;
  SetGenericFontFamilyMap(sans_serif_font_family_map_, family, script);
  return true;
}

const AtomicString& GenericFontFamilySettings::Cursive(
    UScriptCode script) const {
  return GenericFontFamilyForScript(cursive_font_family_map_, script);
}

bool GenericFontFamilySettings::UpdateCursive(const AtomicString& family,
                                              UScriptCode script) {
  if (family == Cursive())
    return false;
  SetGenericFontFamilyMap(cursive_font_family_map_, family, script);
  return true;
}

const AtomicString& GenericFontFamilySettings::Fantasy(
    UScriptCode script) const {
  return GenericFontFamilyForScript(fantasy_font_family_map_, script);
}

bool GenericFontFamilySettings::UpdateFantasy(const AtomicString& family,
                                              UScriptCode script) {
  if (family == Fantasy())
    return false;
  SetGenericFontFamilyMap(fantasy_font_family_map_, family, script);
  return true;
}

const AtomicString& GenericFontFamilySettings::Pictograph(
    UScriptCode script) const {
  return GenericFontFamilyForScript(pictograph_font_family_map_, script);
}

bool GenericFontFamilySettings::UpdatePictograph(const AtomicString& family,
                                                 UScriptCode script) {
  if (family == Pictograph())
    return false;
  SetGenericFontFamilyMap(pictograph_font_family_map_, family, script);
  return true;
}

void GenericFontFamilySettings::Reset() {
  standard_font_family_map_.clear();
  serif_font_family_map_.clear();
  fixed_font_family_map_.clear();
  sans_serif_font_family_map_.clear();
  cursive_font_family_map_.clear();
  fantasy_font_family_map_.clear();
  pictograph_font_family_map_.clear();
}

}  // namespace blink

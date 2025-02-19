// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "skia/ext/skia_utils_base.h"

#include <stdint.h>

#include "base/pickle.h"

namespace skia {

bool ReadSkString(base::PickleIterator* iter, SkString* str) {
  int reply_length;
  const char* reply_text;

  if (!iter->ReadData(&reply_text, &reply_length))
    return false;

  if (str)
    str->set(reply_text, reply_length);
  return true;
}

bool ReadSkFontIdentity(base::PickleIterator* iter,
                        SkFontConfigInterface::FontIdentity* identity) {
  uint32_t reply_id;
  uint32_t reply_ttcIndex;
  int reply_length;
  const char* reply_text;

  if (!iter->ReadUInt32(&reply_id) ||
      !iter->ReadUInt32(&reply_ttcIndex) ||
      !iter->ReadData(&reply_text, &reply_length))
    return false;

  if (identity) {
    identity->fID = reply_id;
    identity->fTTCIndex = reply_ttcIndex;
    identity->fString.set(reply_text, reply_length);
  }
  return true;
}

bool ReadSkFontStyle(base::PickleIterator* iter, SkFontStyle* style) {
  uint16_t reply_weight;
  uint16_t reply_width;
  uint16_t reply_slant;

  if (!iter->ReadUInt16(&reply_weight) ||
      !iter->ReadUInt16(&reply_width) ||
      !iter->ReadUInt16(&reply_slant))
    return false;

  if (style) {
    *style = SkFontStyle(reply_weight,
                         reply_width,
                         static_cast<SkFontStyle::Slant>(reply_slant));
  }
  return true;
}

bool WriteSkString(base::Pickle* pickle, const SkString& str) {
  return pickle->WriteData(str.c_str(), str.size());
}

bool WriteSkFontIdentity(base::Pickle* pickle,
                         const SkFontConfigInterface::FontIdentity& identity) {
  return pickle->WriteUInt32(identity.fID) &&
         pickle->WriteUInt32(identity.fTTCIndex) &&
         WriteSkString(pickle, identity.fString);
}

bool WriteSkFontStyle(base::Pickle* pickle, SkFontStyle style) {
  return pickle->WriteUInt16(style.weight()) &&
         pickle->WriteUInt16(style.width()) &&
         pickle->WriteUInt16(style.slant());
}

}  // namespace skia

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/fonts/opentype/FontSettings.h"

#include "platform/wtf/HashFunctions.h"
#include "platform/wtf/StringHasher.h"
#include "platform/wtf/text/AtomicStringHash.h"
#include "platform/wtf/text/StringHash.h"

namespace blink {

uint32_t AtomicStringToFourByteTag(AtomicString tag) {
  DCHECK_EQ(tag.length(), 4u);
  return (((tag[0]) << 24) | ((tag[1]) << 16) | ((tag[2]) << 8) | (tag[3]));
}

unsigned FontVariationSettings::GetHash() const {
  unsigned computed_hash = size() ? 5381 : 0;
  unsigned num_features = size();
  for (unsigned i = 0; i < num_features; ++i) {
    StringHasher string_hasher;
    const AtomicString& tag = at(i).Tag();
    for (unsigned j = 0; j < tag.length(); j++) {
      string_hasher.AddCharacter(tag[j]);
    }
    WTF::AddIntToHash(computed_hash, string_hasher.GetHash());
    WTF::AddFloatToHash(computed_hash, at(i).Value());
  }
  return computed_hash;
}

}  // namespace blink

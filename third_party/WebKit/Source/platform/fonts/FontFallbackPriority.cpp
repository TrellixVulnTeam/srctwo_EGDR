// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/fonts/FontFallbackPriority.h"

namespace blink {

bool IsNonTextFallbackPriority(FontFallbackPriority fallback_priority) {
  return fallback_priority == FontFallbackPriority::kEmojiText ||
         fallback_priority == FontFallbackPriority::kEmojiEmoji;
};

}  // namespace blink

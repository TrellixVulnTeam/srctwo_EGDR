// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/testing/MockHyphenation.h"

namespace blink {

size_t MockHyphenation::LastHyphenLocation(const StringView& text,
                                           size_t before_index) const {
  String str = text.ToString();
  if (str.EndsWithIgnoringASCIICase("phenation")) {
    if (before_index > 4 + (str.length() - 9))
      return 4 + (str.length() - 9);
    if (str.EndsWithIgnoringASCIICase("hyphenation") &&
        before_index > 2 + (str.length() - 11)) {
      return 2 + (str.length() - 11);
    }
  }
  return 0;
}

}  // namespace blink

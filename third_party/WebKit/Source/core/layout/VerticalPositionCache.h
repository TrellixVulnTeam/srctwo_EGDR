/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef VerticalPositionCache_h
#define VerticalPositionCache_h

#include "core/layout/api/LineLayoutItem.h"
#include "platform/fonts/FontBaseline.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/HashMap.h"

namespace blink {

// Values for vertical alignment.
const int kPositionUndefined = 0x80000000;

class VerticalPositionCache {
  STACK_ALLOCATED();
  WTF_MAKE_NONCOPYABLE(VerticalPositionCache);

 public:
  VerticalPositionCache() {}

  int Get(LineLayoutItem layout_object, FontBaseline baseline_type) const {
    const HashMap<LineLayoutItem, int>& map_to_check =
        baseline_type == kAlphabeticBaseline ? alphabetic_positions_
                                             : ideographic_positions_;
    const HashMap<LineLayoutItem, int>::const_iterator it =
        map_to_check.find(layout_object);
    if (it == map_to_check.end())
      return kPositionUndefined;
    return it->value;
  }

  void Set(LineLayoutItem layout_object,
           FontBaseline baseline_type,
           int position) {
    if (baseline_type == kAlphabeticBaseline)
      alphabetic_positions_.Set(layout_object, position);
    else
      ideographic_positions_.Set(layout_object, position);
  }

 private:
  HashMap<LineLayoutItem, int> alphabetic_positions_;
  HashMap<LineLayoutItem, int> ideographic_positions_;
};

}  // namespace blink

#endif  // VerticalPositionCache_h

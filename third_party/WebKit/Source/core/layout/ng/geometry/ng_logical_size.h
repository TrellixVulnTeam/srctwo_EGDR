// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NGLogicalSize_h
#define NGLogicalSize_h

#include "core/CoreExport.h"
#include "core/layout/ng/geometry/ng_box_strut.h"
#include "core/layout/ng/ng_writing_mode.h"
#include "platform/LayoutUnit.h"

namespace blink {

struct NGPhysicalSize;
#define NGSizeIndefinite LayoutUnit(-1)

// NGLogicalSize is the size of rect (typically a fragment) in the logical
// coordinate system.
struct CORE_EXPORT NGLogicalSize {
  NGLogicalSize() {}
  NGLogicalSize(LayoutUnit inline_size, LayoutUnit block_size)
      : inline_size(inline_size), block_size(block_size) {}

  LayoutUnit inline_size;
  LayoutUnit block_size;

  NGPhysicalSize ConvertToPhysical(NGWritingMode mode) const;
  bool operator==(const NGLogicalSize& other) const;

  bool IsEmpty() const {
    return inline_size == LayoutUnit() || block_size == LayoutUnit();
  }

  void Flip() { std::swap(inline_size, block_size); }
};

inline NGLogicalSize& operator-=(NGLogicalSize& a, const NGBoxStrut& b) {
  a.inline_size -= b.InlineSum();
  a.block_size -= b.BlockSum();
  return a;
}

CORE_EXPORT std::ostream& operator<<(std::ostream&, const NGLogicalSize&);

}  // namespace blink

#endif  // NGLogicalSize_h

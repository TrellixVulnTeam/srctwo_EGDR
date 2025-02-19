// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DecorationInfo_h
#define DecorationInfo_h

#include "platform/LayoutUnit.h"
#include "platform/fonts/FontBaseline.h"
#include "platform/geometry/FloatPoint.h"
#include "platform/wtf/Allocator.h"

namespace blink {

class ComputedStyle;
class SimpleFontData;

enum class ResolvedUnderlinePosition { kRoman, kUnder, kOver };

// Holds text decoration painting values to be computed once and subsequently
// use multiple times to handle decoration paint order correctly. See also
// https://www.w3.org/TR/css-text-decor-3/#painting-order
struct DecorationInfo final {
  STACK_ALLOCATED();

  LayoutUnit width;
  FloatPoint local_origin;
  bool antialias;
  float baseline;
  const ComputedStyle* style;
  const SimpleFontData* font_data;
  float thickness;
  float double_offset;
  FontBaseline baseline_type;
  ResolvedUnderlinePosition underline_position;
};

}  // namespace blink

#endif  // DecorationInfo_h

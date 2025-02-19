// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NGLineHeightMetrics_h
#define NGLineHeightMetrics_h

#include "platform/LayoutUnit.h"
#include "platform/fonts/FontBaseline.h"

namespace blink {

class ComputedStyle;
class FontMetrics;

// Represents line-progression metrics for line boxes and inline boxes.
// Computed for inline boxes, then the metrics of inline boxes are united to
// compute metrics for line boxes.
// https://drafts.csswg.org/css2/visudet.html#line-height
struct NGLineHeightMetrics {
  NGLineHeightMetrics()
      : ascent(LayoutUnit::Min()), descent(LayoutUnit::Min()) {}
  NGLineHeightMetrics(LayoutUnit initial_ascent, LayoutUnit initial_descent)
      : ascent(initial_ascent), descent(initial_descent) {}

  // Compute from ComputedStyle, using the font metrics of the prikmary font.
  // The leading is not included.
  NGLineHeightMetrics(const ComputedStyle&, FontBaseline);

  // Compute from FontMetrics. The leading is not included.
  NGLineHeightMetrics(const FontMetrics&, FontBaseline);

  bool IsEmpty() const { return ascent == LayoutUnit::Min(); }

  // Add the leading. Half the leading is added to ascent and descent each.
  // https://drafts.csswg.org/css2/visudet.html#leading
  void AddLeading(LayoutUnit line_height);

  // Move the metrics by the specified amount, in line progression direction.
  void Move(LayoutUnit);

  // Unite a metrics for an inline box to a metrics for a line box.
  void Unite(const NGLineHeightMetrics&);

  // Ascent and descent of glyphs, or synthesized for replaced elements.
  // Then united to compute 'text-top' and 'text-bottom' of line boxes.
  LayoutUnit ascent;
  LayoutUnit descent;

  LayoutUnit LineHeight() const { return ascent + descent; }

 private:
  void Initialize(const FontMetrics&, FontBaseline);
};

}  // namespace blink

#endif  // NGLineHeightMetrics_h

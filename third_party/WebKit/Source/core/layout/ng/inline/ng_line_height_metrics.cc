// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/layout/ng/inline/ng_line_height_metrics.h"

#include "core/style/ComputedStyle.h"

namespace blink {

NGLineHeightMetrics::NGLineHeightMetrics(const ComputedStyle& style,
                                         FontBaseline baseline_type) {
  const SimpleFontData* font_data = style.GetFont().PrimaryFont();
  DCHECK(font_data);
  Initialize(font_data->GetFontMetrics(), baseline_type);
}

NGLineHeightMetrics::NGLineHeightMetrics(const FontMetrics& font_metrics,
                                         FontBaseline baseline_type) {
  Initialize(font_metrics, baseline_type);
}

void NGLineHeightMetrics::Initialize(const FontMetrics& font_metrics,
                                     FontBaseline baseline_type) {
  ascent = font_metrics.FixedAscent(baseline_type);
  descent = font_metrics.FixedDescent(baseline_type);
}

void NGLineHeightMetrics::AddLeading(LayoutUnit line_height) {
  DCHECK(!IsEmpty());
  LayoutUnit half_leading = (line_height - (ascent + descent)) / 2;
  // TODO(kojii): floor() is to make text dump compatible with legacy test
  // results. Revisit when we paint.
  ascent += half_leading.Floor();
  descent = line_height - ascent;
}

void NGLineHeightMetrics::Move(LayoutUnit delta) {
  DCHECK(!IsEmpty());
  ascent -= delta;
  descent += delta;
}

void NGLineHeightMetrics::Unite(const NGLineHeightMetrics& other) {
  ascent = std::max(ascent, other.ascent);
  descent = std::max(descent, other.descent);
}

}  // namespace blink

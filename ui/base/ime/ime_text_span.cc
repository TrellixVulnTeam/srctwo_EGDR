// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/ime/ime_text_span.h"

#include <string>
#include <vector>

namespace ui {

ImeTextSpan::ImeTextSpan() : ImeTextSpan(0, 0, SK_ColorTRANSPARENT, false) {}

ImeTextSpan::ImeTextSpan(uint32_t start_offset,
                         uint32_t end_offset,
                         SkColor underline_color,
                         bool thick)
    : ImeTextSpan(Type::kComposition,
                  start_offset,
                  end_offset,
                  underline_color,
                  thick,
                  SK_ColorTRANSPARENT) {}

ImeTextSpan::ImeTextSpan(Type type,
                         uint32_t start_offset,
                         uint32_t end_offset,
                         SkColor underline_color,
                         bool thick,
                         SkColor background_color,
                         SkColor suggestion_highlight_color,
                         const std::vector<std::string>& suggestions)
    : type(type),
      start_offset(start_offset),
      end_offset(end_offset),
      underline_color(underline_color),
      thick(thick),
      background_color(background_color),
      suggestion_highlight_color(suggestion_highlight_color),
      suggestions(suggestions) {}

ImeTextSpan::ImeTextSpan(const ImeTextSpan& rhs) = default;

ImeTextSpan::~ImeTextSpan() = default;

}  // namespace ui

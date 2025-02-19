// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/vr/elements/render_text_wrapper.h"

namespace vr {

RenderTextWrapper::RenderTextWrapper(gfx::RenderText* render_text)
    : render_text_(render_text) {}

RenderTextWrapper::~RenderTextWrapper() = default;

void RenderTextWrapper::SetColor(SkColor value) {
  render_text_->SetColor(value);
}

void RenderTextWrapper::ApplyColor(SkColor value, const gfx::Range& range) {
  render_text_->ApplyColor(value, range);
}

void RenderTextWrapper::SetStyle(gfx::TextStyle style, bool value) {
  render_text_->SetStyle(style, value);
}

void RenderTextWrapper::ApplyStyle(gfx::TextStyle style,
                                   bool value,
                                   const gfx::Range& range) {
  render_text_->ApplyStyle(style, value, range);
}

void RenderTextWrapper::SetStrikeThicknessFactor(SkScalar factor) {
  render_text_->set_strike_thickness_factor(factor);
}

}  // namespace vr

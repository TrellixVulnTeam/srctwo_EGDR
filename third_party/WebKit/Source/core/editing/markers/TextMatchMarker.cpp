// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/editing/markers/TextMatchMarker.h"

namespace blink {

TextMatchMarker::TextMatchMarker(unsigned start_offset,
                                 unsigned end_offset,
                                 MatchStatus status)
    : DocumentMarker(start_offset, end_offset), match_status_(status) {}

DocumentMarker::MarkerType TextMatchMarker::GetType() const {
  return DocumentMarker::kTextMatch;
}

bool TextMatchMarker::IsActiveMatch() const {
  return match_status_ == MatchStatus::kActive;
}

void TextMatchMarker::SetIsActiveMatch(bool active) {
  match_status_ = active ? MatchStatus::kActive : MatchStatus::kInactive;
}

bool TextMatchMarker::IsRendered() const {
  return layout_status_ == LayoutStatus::kValidNotNull;
}

bool TextMatchMarker::Contains(const LayoutPoint& point) const {
  DCHECK_EQ(layout_status_, LayoutStatus::kValidNotNull);
  return layout_rect_.Contains(point);
}

void TextMatchMarker::SetLayoutRect(const LayoutRect& rect) {
  if (layout_status_ == LayoutStatus::kValidNotNull && rect == layout_rect_)
    return;
  layout_status_ = LayoutStatus::kValidNotNull;
  layout_rect_ = rect;
}

const LayoutRect& TextMatchMarker::GetLayoutRect() const {
  DCHECK_EQ(layout_status_, LayoutStatus::kValidNotNull);
  return layout_rect_;
}

void TextMatchMarker::NullifyLayoutRect() {
  layout_status_ = LayoutStatus::kValidNull;
  // Now |rendered_rect_| can not be accessed until |SetRenderedRect| is
  // called.
}

void TextMatchMarker::Invalidate() {
  layout_status_ = LayoutStatus::kInvalid;
}

bool TextMatchMarker::IsValid() const {
  return layout_status_ != LayoutStatus::kInvalid;
}

}  // namespace blink

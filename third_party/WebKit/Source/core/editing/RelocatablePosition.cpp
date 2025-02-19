// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/editing/RelocatablePosition.h"

namespace blink {

RelocatablePosition::RelocatablePosition(const Position& position)
    : range_(position.IsNotNull()
                 ? Range::Create(*position.GetDocument(), position, position)
                 : nullptr) {}

RelocatablePosition::~RelocatablePosition() {
  if (!range_)
    return;
  range_->Dispose();
}

Position RelocatablePosition::GetPosition() const {
  if (!range_)
    return Position();
  DCHECK(range_->collapsed());
  return range_->StartPosition();
}

}  // namespace blink

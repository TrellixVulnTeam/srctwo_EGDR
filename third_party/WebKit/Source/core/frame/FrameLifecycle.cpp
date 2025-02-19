// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/frame/FrameLifecycle.h"

#include "platform/wtf/Assertions.h"

namespace blink {

FrameLifecycle::FrameLifecycle() : state_(kAttached) {}

void FrameLifecycle::AdvanceTo(State state) {
  switch (state) {
    case kAttached:
    case kDetached:
      // Normally, only allow state to move forward.
      DCHECK_GT(state, state_);
      break;
    case kDetaching:
      // We can go from Detaching to Detaching since the detach() method can be
      // re-entered.
      DCHECK_GE(state, state_);
      break;
  }
  state_ = state;
}

}  // namespace blink

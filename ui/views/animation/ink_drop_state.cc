// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/animation/ink_drop_state.h"

#include <string>

#include "base/logging.h"

namespace views {

std::string ToString(InkDropState state) {
  switch (state) {
    case InkDropState::HIDDEN:
      return std::string("HIDDEN");
    case InkDropState::ACTION_PENDING:
      return std::string("ACTION_PENDING");
    case InkDropState::ACTION_TRIGGERED:
      return std::string("ACTION_TRIGGERED");
    case InkDropState::ALTERNATE_ACTION_PENDING:
      return std::string("ALTERNATE_ACTION_PENDING");
    case InkDropState::ALTERNATE_ACTION_TRIGGERED:
      return std::string("ALTERNATE_ACTION_TRIGGERED");
    case InkDropState::ACTIVATED:
      return std::string("ACTIVATED");
    case InkDropState::DEACTIVATED:
      return std::string("DEACTIVATED");
  }
  NOTREACHED()
      << "Should never be reached but is necessary for some compilers.";
  return std::string("UNKNOWN");
}

}  // namespace views

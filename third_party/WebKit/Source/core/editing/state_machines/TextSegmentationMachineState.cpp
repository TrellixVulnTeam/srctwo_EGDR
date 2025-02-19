// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/editing/state_machines/TextSegmentationMachineState.h"

#include <ostream>  // NOLINT
#include "platform/wtf/Assertions.h"

namespace blink {

std::ostream& operator<<(std::ostream& os, TextSegmentationMachineState state) {
  static const char* const kTexts[] = {
      "Invalid", "NeedMoreCodeUnit", "NeedFollowingCodeUnit", "Finished",
  };

  const auto& it = std::begin(kTexts) + static_cast<size_t>(state);
  DCHECK_GE(it, std::begin(kTexts)) << "Unknown state value";
  DCHECK_LT(it, std::end(kTexts)) << "Unknown state value";
  return os << *it;
}

}  // namespace blink

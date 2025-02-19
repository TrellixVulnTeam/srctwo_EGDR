// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/editing/TextAffinity.h"

#include <ostream>  // NOLINT
#include "platform/wtf/Assertions.h"
#include "public/web/WebAXEnums.h"

namespace blink {

std::ostream& operator<<(std::ostream& ostream, TextAffinity affinity) {
  switch (affinity) {
    case TextAffinity::kDownstream:
      return ostream << "TextAffinity::Downstream";
    case TextAffinity::kUpstream:
      return ostream << "TextAffinity::Upstream";
  }
  return ostream << "TextAffinity(" << static_cast<int>(affinity) << ')';
}


STATIC_ASSERT_ENUM(kWebAXTextAffinityUpstream, TextAffinity::kUpstream);
STATIC_ASSERT_ENUM(kWebAXTextAffinityDownstream, TextAffinity::kDownstream);

}  // namespace blink

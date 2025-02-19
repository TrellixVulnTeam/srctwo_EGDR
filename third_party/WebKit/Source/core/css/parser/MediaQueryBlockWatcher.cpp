// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/css/parser/MediaQueryBlockWatcher.h"

#include "core/css/parser/CSSParserToken.h"

namespace blink {

MediaQueryBlockWatcher::MediaQueryBlockWatcher() : block_level_(0) {}

void MediaQueryBlockWatcher::HandleToken(const CSSParserToken& token) {
  if (token.GetBlockType() == CSSParserToken::kBlockStart) {
    ++block_level_;
  } else if (token.GetBlockType() == CSSParserToken::kBlockEnd) {
    DCHECK(block_level_);
    --block_level_;
  }
}

}  // namespace blink

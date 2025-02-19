// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/editing/serializers/TextOffset.h"

#include "core/dom/Text.h"

namespace blink {

TextOffset::TextOffset() : offset_(0) {}

TextOffset::TextOffset(Text* text, int offset) : text_(text), offset_(offset) {}

TextOffset::TextOffset(const TextOffset& other)
    : text_(other.text_), offset_(other.offset_) {}

bool TextOffset::IsNull() const {
  return !text_;
}

bool TextOffset::IsNotNull() const {
  return text_;
}

}  // namespace blink

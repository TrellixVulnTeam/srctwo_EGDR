// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/dom/StaticRange.h"

#include "bindings/core/v8/ExceptionState.h"
#include "core/dom/Range.h"
#include "core/frame/LocalDOMWindow.h"
#include "platform/bindings/ScriptState.h"

namespace blink {

StaticRange::StaticRange(Document& document)
    : owner_document_(document),
      start_container_(document),
      start_offset_(0u),
      end_container_(document),
      end_offset_(0u) {}

StaticRange::StaticRange(Document& document,
                         Node* start_container,
                         unsigned start_offset,
                         Node* end_container,
                         unsigned end_offset)
    : owner_document_(document),
      start_container_(start_container),
      start_offset_(start_offset),
      end_container_(end_container),
      end_offset_(end_offset) {}

void StaticRange::setStart(Node* container, unsigned offset) {
  start_container_ = container;
  start_offset_ = offset;
}

void StaticRange::setEnd(Node* container, unsigned offset) {
  end_container_ = container;
  end_offset_ = offset;
}

Range* StaticRange::toRange(ExceptionState& exception_state) const {
  Range* range = Range::Create(*owner_document_.Get());
  // Do the offset checking.
  range->setStart(start_container_, start_offset_, exception_state);
  range->setEnd(end_container_, end_offset_, exception_state);
  return range;
}

DEFINE_TRACE(StaticRange) {
  visitor->Trace(owner_document_);
  visitor->Trace(start_container_);
  visitor->Trace(end_container_);
}

}  // namespace blink

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef StaticRange_h
#define StaticRange_h

#include "core/CoreExport.h"
#include "core/dom/Range.h"
#include "core/editing/EphemeralRange.h"
#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/Handle.h"

namespace blink {

class Document;
class ExceptionState;

class CORE_EXPORT StaticRange final : public GarbageCollected<StaticRange>,
                                      public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static StaticRange* Create(Document& document) {
    return new StaticRange(document);
  }
  static StaticRange* Create(Document& document,
                             Node* start_container,
                             unsigned start_offset,
                             Node* end_container,
                             unsigned end_offset) {
    return new StaticRange(document, start_container, start_offset,
                           end_container, end_offset);
  }
  static StaticRange* Create(const Range* range) {
    return new StaticRange(range->OwnerDocument(), range->startContainer(),
                           range->startOffset(), range->endContainer(),
                           range->endOffset());
  }
  static StaticRange* Create(const EphemeralRange& range) {
    DCHECK(!range.IsNull());
    return new StaticRange(range.GetDocument(),
                           range.StartPosition().ComputeContainerNode(),
                           range.StartPosition().ComputeOffsetInContainerNode(),
                           range.EndPosition().ComputeContainerNode(),
                           range.EndPosition().ComputeOffsetInContainerNode());
  }

  Node* startContainer() const { return start_container_.Get(); }
  void setStartContainer(Node* start_container) {
    start_container_ = start_container;
  }

  unsigned startOffset() const { return start_offset_; }
  void setStartOffset(unsigned start_offset) { start_offset_ = start_offset; }

  Node* endContainer() const { return end_container_.Get(); }
  void setEndContainer(Node* end_container) { end_container_ = end_container; }

  unsigned endOffset() const { return end_offset_; }
  void setEndOffset(unsigned end_offset) { end_offset_ = end_offset; }

  bool collapsed() const {
    return start_container_ == end_container_ && start_offset_ == end_offset_;
  }

  void setStart(Node* container, unsigned offset);
  void setEnd(Node* container, unsigned offset);

  Range* toRange(ExceptionState& = ASSERT_NO_EXCEPTION) const;

  DECLARE_TRACE();

 private:
  explicit StaticRange(Document&);
  StaticRange(Document&,
              Node* start_container,
              unsigned start_offset,
              Node* end_container,
              unsigned end_offset);

  Member<Document> owner_document_;  // Required by |toRange()|.
  Member<Node> start_container_;
  unsigned start_offset_;
  Member<Node> end_container_;
  unsigned end_offset_;
};

using StaticRangeVector = HeapVector<Member<StaticRange>>;

}  // namespace blink

#endif  // StaticRange_h

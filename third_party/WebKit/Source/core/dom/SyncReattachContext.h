// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SyncReattachContext_h
#define SyncReattachContext_h

#include "core/dom/Node.h"

namespace blink {

class SyncReattachContext {
  STACK_ALLOCATED();

 public:
  static Node::AttachContext& CurrentAttachContext() {
    DCHECK(current_context_);
    return current_context_->AttachContextForReattach();
  }
  SyncReattachContext(Node::AttachContext& attach_context)
      : parent_context_(current_context_),
        previous_in_flow_(attach_context.previous_in_flow),
        attach_context_(attach_context) {
    current_context_ = this;
  }
  ~SyncReattachContext() { current_context_ = parent_context_; }

 private:
  Node::AttachContext& AttachContextForReattach() {
    attach_context_.previous_in_flow = previous_in_flow_;
    return attach_context_;
  }

  static SyncReattachContext* current_context_;

  SyncReattachContext* parent_context_ = nullptr;
  LayoutObject* previous_in_flow_;
  Node::AttachContext& attach_context_;
};

}  // namespace blink

#endif  // SyncReattachContext_h

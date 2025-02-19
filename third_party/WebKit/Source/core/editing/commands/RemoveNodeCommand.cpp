/*
 * Copyright (C) 2005, 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "core/editing/commands/RemoveNodeCommand.h"

#include "bindings/core/v8/ExceptionState.h"
#include "core/dom/Node.h"
#include "core/editing/EditingUtilities.h"
#include "core/editing/commands/EditingState.h"
#include "platform/wtf/Assertions.h"

namespace blink {

RemoveNodeCommand::RemoveNodeCommand(
    Node* node,
    ShouldAssumeContentIsAlwaysEditable
        should_assume_content_is_always_editable)
    : SimpleEditCommand(node->GetDocument()),
      node_(node),
      should_assume_content_is_always_editable_(
          should_assume_content_is_always_editable) {
  DCHECK(node_);
  DCHECK(node_->parentNode());
}

void RemoveNodeCommand::DoApply(EditingState* editing_state) {
  ContainerNode* parent = node_->parentNode();
  GetDocument().UpdateStyleAndLayoutTree();
  if (!parent || (should_assume_content_is_always_editable_ ==
                      kDoNotAssumeContentIsAlwaysEditable &&
                  !HasEditableStyle(*parent) && parent->InActiveDocument()))
    return;
  DCHECK(HasEditableStyle(*parent) || !parent->InActiveDocument()) << parent;

  parent_ = parent;
  ref_child_ = node_->nextSibling();

  node_->remove(IGNORE_EXCEPTION_FOR_TESTING);
  // Node::remove dispatch synchronous events such as IFRAME unload events,
  // and event handlers may break the document. We check the document state
  // here in order to prevent further processing in bad situation.
  ABORT_EDITING_COMMAND_IF(!node_->GetDocument().GetFrame());
  ABORT_EDITING_COMMAND_IF(!node_->GetDocument().documentElement());
}

void RemoveNodeCommand::DoUnapply() {
  ContainerNode* parent = parent_.Release();
  Node* ref_child = ref_child_.Release();
  if (!parent || !HasEditableStyle(*parent))
    return;

  parent->InsertBefore(node_.Get(), ref_child, IGNORE_EXCEPTION_FOR_TESTING);
}

DEFINE_TRACE(RemoveNodeCommand) {
  visitor->Trace(node_);
  visitor->Trace(parent_);
  visitor->Trace(ref_child_);
  SimpleEditCommand::Trace(visitor);
}

}  // namespace blink

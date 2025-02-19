/*
 * Copyright (C) 2005, 2006, 2007 Apple, Inc.  All rights reserved.
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

#include "core/editing/commands/EditCommand.h"

#include "core/dom/Document.h"
#include "core/dom/NodeTraversal.h"
#include "core/editing/FrameSelection.h"
#include "core/editing/commands/CompositeEditCommand.h"
#include "core/frame/LocalFrame.h"
#include "core/layout/LayoutText.h"

namespace blink {

EditCommand::EditCommand(Document& document)
    : document_(&document), parent_(nullptr) {
  DCHECK(document_);
  DCHECK(document_->GetFrame());
}

EditCommand::~EditCommand() {}

InputEvent::InputType EditCommand::GetInputType() const {
  return InputEvent::InputType::kNone;
}

String EditCommand::TextDataForInputEvent() const {
  return g_null_atom;
}

bool EditCommand::IsRenderedCharacter(const Position& position) {
  if (position.IsNull())
    return false;
  DCHECK(position.IsOffsetInAnchor()) << position;
  if (!position.AnchorNode()->IsTextNode())
    return false;

  LayoutObject* layout_object = position.AnchorNode()->GetLayoutObject();
  if (!layout_object)
    return false;

  return ToLayoutText(layout_object)
      ->IsRenderedCharacter(position.OffsetInContainerNode());
}

void EditCommand::SetParent(CompositeEditCommand* parent) {
  DCHECK((parent && !parent_) || (!parent && parent_));
  DCHECK(!parent || !IsCompositeEditCommand() ||
         !ToCompositeEditCommand(this)->GetUndoStep());
  parent_ = parent;
}

void SimpleEditCommand::DoReapply() {
  EditingState editing_state;
  DoApply(&editing_state);
}

DEFINE_TRACE(EditCommand) {
  visitor->Trace(document_);
  visitor->Trace(parent_);
}

}  // namespace blink

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

#include "core/editing/commands/SplitTextNodeCommand.h"

#include "bindings/core/v8/ExceptionState.h"
#include "core/dom/Document.h"
#include "core/dom/Text.h"
#include "core/editing/EditingUtilities.h"
#include "core/editing/markers/DocumentMarkerController.h"
#include "platform/wtf/Assertions.h"

namespace blink {

SplitTextNodeCommand::SplitTextNodeCommand(Text* text, int offset)
    : SimpleEditCommand(text->GetDocument()), text2_(text), offset_(offset) {
  // NOTE: Various callers rely on the fact that the original node becomes
  // the second node (i.e. the new node is inserted before the existing one).
  // That is not a fundamental dependency (i.e. it could be re-coded), but
  // rather is based on how this code happens to work.
  DCHECK(text2_);
  DCHECK_GT(text2_->length(), 0u);
  DCHECK_GT(offset_, 0u);
  DCHECK_LT(offset_, text2_->length());
}

void SplitTextNodeCommand::DoApply(EditingState*) {
  ContainerNode* parent = text2_->parentNode();
  if (!parent || !HasEditableStyle(*parent))
    return;

  String prefix_text =
      text2_->substringData(0, offset_, IGNORE_EXCEPTION_FOR_TESTING);
  if (prefix_text.IsEmpty())
    return;

  text1_ = Text::Create(GetDocument(), prefix_text);
  DCHECK(text1_);
  GetDocument().Markers().MoveMarkers(text2_.Get(), offset_, text1_.Get());

  InsertText1AndTrimText2();
}

void SplitTextNodeCommand::DoUnapply() {
  if (!text1_ || !HasEditableStyle(*text1_))
    return;

  DCHECK_EQ(text1_->GetDocument(), GetDocument());

  String prefix_text = text1_->data();

  text2_->insertData(0, prefix_text, ASSERT_NO_EXCEPTION);
  GetDocument().UpdateStyleAndLayout();

  GetDocument().Markers().MoveMarkers(text1_.Get(), prefix_text.length(),
                                      text2_.Get());
  text1_->remove(ASSERT_NO_EXCEPTION);
}

void SplitTextNodeCommand::DoReapply() {
  if (!text1_ || !text2_)
    return;

  ContainerNode* parent = text2_->parentNode();
  if (!parent || !HasEditableStyle(*parent))
    return;

  GetDocument().Markers().MoveMarkers(text2_.Get(), offset_, text1_.Get());

  InsertText1AndTrimText2();
}

void SplitTextNodeCommand::InsertText1AndTrimText2() {
  DummyExceptionStateForTesting exception_state;
  text2_->parentNode()->InsertBefore(text1_.Get(), text2_.Get(),
                                     exception_state);
  if (exception_state.HadException())
    return;
  text2_->deleteData(0, offset_, exception_state);
  GetDocument().UpdateStyleAndLayout();
}

DEFINE_TRACE(SplitTextNodeCommand) {
  visitor->Trace(text1_);
  visitor->Trace(text2_);
  SimpleEditCommand::Trace(visitor);
}

}  // namespace blink

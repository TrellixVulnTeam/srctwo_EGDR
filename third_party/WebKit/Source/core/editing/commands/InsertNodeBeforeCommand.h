/*
 * Copyright (C) 2005, 2006, 2008 Apple Inc. All rights reserved.
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

#ifndef InsertNodeBeforeCommand_h
#define InsertNodeBeforeCommand_h

#include "core/editing/commands/EditCommand.h"

namespace blink {

class InsertNodeBeforeCommand final : public SimpleEditCommand {
 public:
  static InsertNodeBeforeCommand* Create(
      Node* child_to_insert,
      Node* child_to_insert_before,
      ShouldAssumeContentIsAlwaysEditable
          should_assume_content_is_always_editable) {
    return new InsertNodeBeforeCommand(
        child_to_insert, child_to_insert_before,
        should_assume_content_is_always_editable);
  }

  DECLARE_VIRTUAL_TRACE();

 private:
  InsertNodeBeforeCommand(Node* child_to_insert,
                          Node* child_to_insert_before,
                          ShouldAssumeContentIsAlwaysEditable);

  void DoApply(EditingState*) override;
  void DoUnapply() override;

  Member<Node> insert_child_;
  Member<Node> ref_child_;
  ShouldAssumeContentIsAlwaysEditable should_assume_content_is_always_editable_;
};

}  // namespace blink

#endif  // InsertNodeBeforeCommand_h

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

#ifndef InsertParagraphSeparatorCommand_h
#define InsertParagraphSeparatorCommand_h

#include "core/editing/commands/CompositeEditCommand.h"

namespace blink {

class EditingStyle;

class InsertParagraphSeparatorCommand final : public CompositeEditCommand {
 public:
  static InsertParagraphSeparatorCommand* Create(
      Document& document,
      bool use_default_paragraph_element = false,
      bool paste_blockquote_into_unquoted_area = false) {
    return new InsertParagraphSeparatorCommand(
        document, use_default_paragraph_element,
        paste_blockquote_into_unquoted_area);
  }

  DECLARE_VIRTUAL_TRACE();

 private:
  InsertParagraphSeparatorCommand(Document&,
                                  bool use_default_paragraph_element,
                                  bool paste_blockquote_into_unquoted_area);

  void DoApply(EditingState*) override;

  void CalculateStyleBeforeInsertion(const Position&);
  void ApplyStyleAfterInsertion(Element* original_enclosing_block,
                                EditingState*);
  void GetAncestorsInsideBlock(const Node* insertion_node,
                               Element* outer_block,
                               HeapVector<Member<Element>>& ancestors);
  Element* CloneHierarchyUnderNewBlock(
      const HeapVector<Member<Element>>& ancestors,
      Element* block_to_insert,
      EditingState*);

  bool ShouldUseDefaultParagraphElement(Element*) const;

  bool PreservesTypingStyle() const override;

  Member<EditingStyle> style_;

  bool must_use_default_paragraph_element_;
  bool paste_blockquote_into_unquoted_area_;
};

}  // namespace blink

#endif

/*
 * Copyright (C) 2006, 2008 Apple Inc. All rights reserved.
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

#ifndef InsertListCommand_h
#define InsertListCommand_h

#include "core/editing/commands/CompositeEditCommand.h"

namespace blink {

class HTMLElement;
class HTMLUListElement;

class CORE_EXPORT InsertListCommand final : public CompositeEditCommand {
 public:
  enum Type { kOrderedList, kUnorderedList };

  static InsertListCommand* Create(Document& document, Type list_type) {
    return new InsertListCommand(document, list_type);
  }

  bool PreservesTypingStyle() const override { return true; }

  DECLARE_VIRTUAL_TRACE();

 private:
  InsertListCommand(Document&, Type);

  void DoApply(EditingState*) override;
  InputEvent::InputType GetInputType() const override;

  HTMLUListElement* FixOrphanedListChild(Node*, EditingState*);
  bool SelectionHasListOfType(const Position& selection_start,
                              const Position& selection_end,
                              const HTMLQualifiedName&);
  HTMLElement* MergeWithNeighboringLists(HTMLElement*, EditingState*);
  bool DoApplyForSingleParagraph(bool force_create_list,
                                 const HTMLQualifiedName&,
                                 Range& current_selection,
                                 EditingState*);
  void UnlistifyParagraph(const VisiblePosition& original_start,
                          HTMLElement* list_node,
                          Node* list_child_node,
                          EditingState*);
  void ListifyParagraph(const VisiblePosition& original_start,
                        const HTMLQualifiedName& list_tag,
                        EditingState*);
  void MoveParagraphOverPositionIntoEmptyListItem(const VisiblePosition&,
                                                  HTMLLIElement*,
                                                  EditingState*);

  Type type_;
};

}  // namespace blink

#endif  // InsertListCommand_h

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

#ifndef EditCommand_h
#define EditCommand_h

#include "core/CoreExport.h"
#include "core/events/InputEvent.h"
#include "platform/heap/Handle.h"

namespace blink {

class CompositeEditCommand;
class Document;
class EditingState;

class CORE_EXPORT EditCommand : public GarbageCollectedFinalized<EditCommand> {
 public:
  virtual ~EditCommand();

  virtual void SetParent(CompositeEditCommand*);

  virtual InputEvent::InputType GetInputType() const;

  virtual bool IsSimpleEditCommand() const { return false; }
  virtual bool IsCompositeEditCommand() const { return false; }
  bool IsTopLevelCommand() const { return !parent_; }

  // The |EditingState*| argument must not be nullptr.
  virtual void DoApply(EditingState*) = 0;

  // |TypingCommand| will return the text of the last |m_commands|.
  virtual String TextDataForInputEvent() const;

  DECLARE_VIRTUAL_TRACE();

 protected:
  explicit EditCommand(Document&);

  Document& GetDocument() const { return *document_.Get(); }
  CompositeEditCommand* Parent() const { return parent_; }

  // TODO(yosin) |isRenderedCharacter()| should be removed, and we should use
  // |VisiblePosition::characterAfter()| and
  // |VisiblePosition::characterBefore()|.
  static bool IsRenderedCharacter(const Position&);

 private:
  Member<Document> document_;
  Member<CompositeEditCommand> parent_;
};

enum ShouldAssumeContentIsAlwaysEditable {
  kAssumeContentIsAlwaysEditable,
  kDoNotAssumeContentIsAlwaysEditable,
};

class CORE_EXPORT SimpleEditCommand : public EditCommand {
 public:
  virtual void DoUnapply() = 0;
  virtual void DoReapply();  // calls doApply()

 protected:
  explicit SimpleEditCommand(Document& document) : EditCommand(document) {}

 private:
  bool IsSimpleEditCommand() const final { return true; }
};

DEFINE_TYPE_CASTS(SimpleEditCommand,
                  EditCommand,
                  command,
                  command->IsSimpleEditCommand(),
                  command.IsSimpleEditCommand());

}  // namespace blink

#endif  // EditCommand_h

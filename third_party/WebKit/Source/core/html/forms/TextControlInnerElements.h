/*
 * Copyright (C) 2006, 2008, 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Google Inc. All rights reserved.
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

#ifndef TextControlInnerElements_h
#define TextControlInnerElements_h

#include "core/html/HTMLDivElement.h"
#include "platform/wtf/Forward.h"

namespace blink {

class TextControlInnerContainer final : public HTMLDivElement {
 public:
  static TextControlInnerContainer* Create(Document&);

 protected:
  explicit TextControlInnerContainer(Document&);
  LayoutObject* CreateLayoutObject(const ComputedStyle&) override;
};

class EditingViewPortElement final : public HTMLDivElement {
 public:
  static EditingViewPortElement* Create(Document&);

 protected:
  explicit EditingViewPortElement(Document&);
  RefPtr<ComputedStyle> CustomStyleForLayoutObject() override;

 private:
  bool SupportsFocus() const override { return false; }
};

class TextControlInnerEditorElement final : public HTMLDivElement {
 public:
  static TextControlInnerEditorElement* Create(Document&);

  void DefaultEventHandler(Event*) override;

 private:
  explicit TextControlInnerEditorElement(Document&);
  LayoutObject* CreateLayoutObject(const ComputedStyle&) override;
  RefPtr<ComputedStyle> CustomStyleForLayoutObject() override;
  bool SupportsFocus() const override { return false; }
};

class SearchFieldCancelButtonElement final : public HTMLDivElement {
 public:
  static SearchFieldCancelButtonElement* Create(Document&);

  void DefaultEventHandler(Event*) override;
  bool WillRespondToMouseClickEvents() override;

 private:
  explicit SearchFieldCancelButtonElement(Document&);
  void DetachLayoutTree(const AttachContext& = AttachContext()) override;
  bool SupportsFocus() const override { return false; }

  bool capturing_;
};

}  // namespace blink

#endif

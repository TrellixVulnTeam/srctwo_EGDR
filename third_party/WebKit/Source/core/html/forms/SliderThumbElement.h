/*
 * Copyright (C) 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SliderThumbElement_h
#define SliderThumbElement_h

#include "core/HTMLNames.h"
#include "core/html/HTMLDivElement.h"
#include "platform/wtf/Forward.h"

namespace blink {

class HTMLInputElement;
class Event;
class TouchEvent;

class SliderThumbElement final : public HTMLDivElement {
 public:
  static SliderThumbElement* Create(Document&);

  void SetPositionFromValue();

  void DragFrom(const LayoutPoint&);
  void DefaultEventHandler(Event*) override;
  bool WillRespondToMouseMoveEvents() override;
  bool WillRespondToMouseClickEvents() override;
  void DetachLayoutTree(const AttachContext& = AttachContext()) override;
  const AtomicString& ShadowPseudoId() const override;
  HTMLInputElement* HostInput() const;
  void SetPositionFromPoint(const LayoutPoint&);
  void StopDragging();

 private:
  SliderThumbElement(Document&);
  LayoutObject* CreateLayoutObject(const ComputedStyle&) override;
  Element* CloneElementWithoutAttributesAndChildren() override;
  bool IsDisabledFormControl() const override;
  bool MatchesReadOnlyPseudoClass() const override;
  bool MatchesReadWritePseudoClass() const override;
  Node* FocusDelegate() override;
  void StartDragging();

  bool
      in_drag_mode_;  // Mouse only. Touch is handled by SliderContainerElement.
};

inline Element* SliderThumbElement::CloneElementWithoutAttributesAndChildren() {
  return Create(GetDocument());
}

// FIXME: There are no ways to check if a node is a SliderThumbElement.
DEFINE_ELEMENT_TYPE_CASTS(SliderThumbElement, IsHTMLElement());

class SliderContainerElement final : public HTMLDivElement {
 public:
  enum Direction {
    kHorizontal,
    kVertical,
    kNoMove,
  };

  DECLARE_NODE_FACTORY(SliderContainerElement);
  HTMLInputElement* HostInput() const;
  void DefaultEventHandler(Event*) override;
  void HandleTouchEvent(TouchEvent*);
  void UpdateTouchEventHandlerRegistry();
  void DidMoveToNewDocument(Document&) override;
  void RemoveAllEventListeners() override;

 private:
  explicit SliderContainerElement(Document&);
  LayoutObject* CreateLayoutObject(const ComputedStyle&) override;
  const AtomicString& ShadowPseudoId() const override;
  Direction GetDirection(LayoutPoint&, LayoutPoint&);
  bool CanSlide();

  bool has_touch_event_handler_;
  bool touch_started_;
  Direction sliding_direction_;
  LayoutPoint start_point_;
};

}  // namespace blink

#endif

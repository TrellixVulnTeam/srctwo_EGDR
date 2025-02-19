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

#ifndef SpinButtonElement_h
#define SpinButtonElement_h

#include "core/CoreExport.h"
#include "core/html/HTMLDivElement.h"
#include "core/page/PopupOpeningObserver.h"
#include "platform/Timer.h"

namespace blink {

class CORE_EXPORT SpinButtonElement final : public HTMLDivElement,
                                            public PopupOpeningObserver {
 public:
  enum UpDownState {
    kIndeterminate,  // Hovered, but the event is not handled.
    kDown,
    kUp,
  };
  enum EventDispatch {
    kEventDispatchAllowed,
    kEventDispatchDisallowed,
  };
  class SpinButtonOwner : public GarbageCollectedMixin {
   public:
    virtual ~SpinButtonOwner() {}
    virtual void FocusAndSelectSpinButtonOwner() = 0;
    virtual bool ShouldSpinButtonRespondToMouseEvents() = 0;
    virtual bool ShouldSpinButtonRespondToWheelEvents() = 0;
    virtual void SpinButtonDidReleaseMouseCapture(EventDispatch) = 0;
    virtual void SpinButtonStepDown() = 0;
    virtual void SpinButtonStepUp() = 0;
  };

  // The owner of SpinButtonElement must call removeSpinButtonOwner
  // because SpinButtonElement can be outlive SpinButtonOwner
  // implementation, e.g. during event handling.
  static SpinButtonElement* Create(Document&, SpinButtonOwner&);
  UpDownState GetUpDownState() const { return up_down_state_; }
  void ReleaseCapture(EventDispatch = kEventDispatchAllowed);
  void RemoveSpinButtonOwner() { spin_button_owner_ = nullptr; }

  void Step(int amount);

  bool WillRespondToMouseMoveEvents() override;
  bool WillRespondToMouseClickEvents() override;

  void ForwardEvent(Event*);

  DECLARE_VIRTUAL_TRACE();

 private:
  SpinButtonElement(Document&, SpinButtonOwner&);

  void DetachLayoutTree(const AttachContext&) override;
  bool IsSpinButtonElement() const override { return true; }
  bool IsDisabledFormControl() const override {
    return OwnerShadowHost() && OwnerShadowHost()->IsDisabledFormControl();
  }
  bool MatchesReadOnlyPseudoClass() const override;
  bool MatchesReadWritePseudoClass() const override;
  void DefaultEventHandler(Event*) override;
  void WillOpenPopup() override;
  void DoStepAction(int);
  void StartRepeatingTimer();
  void StopRepeatingTimer();
  void RepeatingTimerFired(TimerBase*);
  void SetHovered(bool = true) override;
  bool ShouldRespondToMouseEvents();
  bool IsMouseFocusable() const override { return false; }

  Member<SpinButtonOwner> spin_button_owner_;
  bool capturing_;
  UpDownState up_down_state_;
  UpDownState press_starting_state_;
  TaskRunnerTimer<SpinButtonElement> repeating_timer_;
};

DEFINE_TYPE_CASTS(SpinButtonElement,
                  Node,
                  node,
                  ToElement(node)->IsSpinButtonElement(),
                  ToElement(node).IsSpinButtonElement());

}  // namespace blink

#endif

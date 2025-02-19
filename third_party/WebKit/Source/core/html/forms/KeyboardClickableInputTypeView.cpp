/*
 * Copyright (C) 2010, 2012 Google Inc. All rights reserved.
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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

#include "core/html/forms/KeyboardClickableInputTypeView.h"

#include "core/events/KeyboardEvent.h"
#include "core/html/HTMLInputElement.h"

namespace blink {

using namespace HTMLNames;

void KeyboardClickableInputTypeView::HandleKeydownEvent(KeyboardEvent* event) {
  const String& key = event->key();
  if (key == " ") {
    GetElement().SetActive(true);
    // No setDefaultHandled(), because IE dispatches a keypress in this case
    // and the caller will only dispatch a keypress if we don't call
    // setDefaultHandled().
  }
}

void KeyboardClickableInputTypeView::HandleKeypressEvent(KeyboardEvent* event) {
  int char_code = event->charCode();
  if (char_code == '\r') {
    GetElement().DispatchSimulatedClick(event);
    event->SetDefaultHandled();
    return;
  }
  if (char_code == ' ') {
    // Prevent scrolling down the page.
    event->SetDefaultHandled();
  }
}

void KeyboardClickableInputTypeView::HandleKeyupEvent(KeyboardEvent* event) {
  const String& key = event->key();
  if (key != " ")
    return;
  // Simulate mouse click for spacebar for button types.
  DispatchSimulatedClickIfActive(event);
}

// FIXME: Could share this with BaseCheckableInputType and RangeInputType if we
// had a common base class.
void KeyboardClickableInputTypeView::AccessKeyAction(bool send_mouse_events) {
  InputTypeView::AccessKeyAction(send_mouse_events);
  GetElement().DispatchSimulatedClick(
      0, send_mouse_events ? kSendMouseUpDownEvents : kSendNoEvents);
}

}  // namespace blink

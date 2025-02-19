/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2013 Intel Corporation. All rights reserved.
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

#include "core/events/TransitionEvent.h"

namespace blink {

TransitionEvent::TransitionEvent() : elapsed_time_(0) {}

TransitionEvent::TransitionEvent(const AtomicString& type,
                                 const String& property_name,
                                 double elapsed_time,
                                 const String& pseudo_element)
    : Event(type, true, true),
      property_name_(property_name),
      elapsed_time_(elapsed_time),
      pseudo_element_(pseudo_element) {}

TransitionEvent::TransitionEvent(const AtomicString& type,
                                 const TransitionEventInit& initializer)
    : Event(type, initializer), elapsed_time_(0) {
  if (initializer.hasPropertyName())
    property_name_ = initializer.propertyName();
  if (initializer.hasElapsedTime())
    elapsed_time_ = initializer.elapsedTime();
  if (initializer.hasPseudoElement())
    pseudo_element_ = initializer.pseudoElement();
}

TransitionEvent::~TransitionEvent() {}

const String& TransitionEvent::propertyName() const {
  return property_name_;
}

double TransitionEvent::elapsedTime() const {
  return elapsed_time_;
}

const String& TransitionEvent::pseudoElement() const {
  return pseudo_element_;
}

const AtomicString& TransitionEvent::InterfaceName() const {
  return EventNames::TransitionEvent;
}

DEFINE_TRACE(TransitionEvent) {
  Event::Trace(visitor);
}

}  // namespace blink

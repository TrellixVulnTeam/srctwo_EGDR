/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
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

#include "core/events/ErrorEvent.h"

#include <memory>

#include "bindings/core/v8/V8BindingForCore.h"
#include "v8/include/v8.h"

namespace blink {

ErrorEvent::ErrorEvent()
    : sanitized_message_(),
      location_(SourceLocation::Create(String(), 0, 0, nullptr)),
      error_(this),
      world_(DOMWrapperWorld::Current(v8::Isolate::GetCurrent())) {}

ErrorEvent::ErrorEvent(ScriptState* script_state,
                       const AtomicString& type,
                       const ErrorEventInit& initializer)
    : Event(type, initializer),
      sanitized_message_(),
      error_(this),
      world_(script_state->World()) {
  if (initializer.hasMessage())
    sanitized_message_ = initializer.message();
  location_ = SourceLocation::Create(
      initializer.hasFilename() ? initializer.filename() : String(),
      initializer.hasLineno() ? initializer.lineno() : 0,
      initializer.hasColno() ? initializer.colno() : 0, nullptr);
  if (initializer.hasError()) {
    error_.Set(initializer.error().GetIsolate(), initializer.error().V8Value());
  }
}

ErrorEvent::ErrorEvent(const String& message,
                       std::unique_ptr<SourceLocation> location,
                       ScriptValue error,
                       DOMWrapperWorld* world)
    : Event(EventTypeNames::error, false, true),
      sanitized_message_(message),
      location_(std::move(location)),
      error_(this),
      world_(world) {
  if (!error.IsEmpty())
    error_.Set(error.GetIsolate(), error.V8Value());
}

void ErrorEvent::SetUnsanitizedMessage(const String& message) {
  DCHECK(unsanitized_message_.IsEmpty());
  unsanitized_message_ = message;
}

ErrorEvent::~ErrorEvent() {}

const AtomicString& ErrorEvent::InterfaceName() const {
  return EventNames::ErrorEvent;
}

ScriptValue ErrorEvent::error(ScriptState* script_state) const {
  // Don't return |m_error| when we are in the different worlds to avoid
  // leaking a V8 value.
  // We do not clone Error objects (exceptions), for 2 reasons:
  // 1) Errors carry a reference to the isolated world's global object, and
  //    thus passing it around would cause leakage.
  // 2) Errors cannot be cloned (or serialized):
  if (World() != &script_state->World() || error_.IsEmpty())
    return ScriptValue();
  return ScriptValue(script_state, error_.NewLocal(script_state->GetIsolate()));
}

DEFINE_TRACE(ErrorEvent) {
  Event::Trace(visitor);
}

DEFINE_TRACE_WRAPPERS(ErrorEvent) {
  visitor->TraceWrappers(error_);
  Event::TraceWrappers(visitor);
}

}  // namespace blink

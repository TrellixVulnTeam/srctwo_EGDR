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

#ifndef ErrorEvent_h
#define ErrorEvent_h

#include <memory>
#include "bindings/core/v8/SourceLocation.h"
#include "core/dom/events/Event.h"
#include "core/events/ErrorEventInit.h"
#include "platform/bindings/DOMWrapperWorld.h"
#include "platform/bindings/TraceWrapperV8Reference.h"
#include "platform/wtf/RefPtr.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class ErrorEvent final : public Event {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static ErrorEvent* Create() { return new ErrorEvent; }
  static ErrorEvent* Create(const String& message,
                            std::unique_ptr<SourceLocation> location,
                            DOMWrapperWorld* world) {
    return new ErrorEvent(message, std::move(location), ScriptValue(), world);
  }

  static ErrorEvent* Create(const String& message,
                            std::unique_ptr<SourceLocation> location,
                            ScriptValue error,
                            DOMWrapperWorld* world) {
    return new ErrorEvent(message, std::move(location), error, world);
  }

  static ErrorEvent* Create(ScriptState* script_state,
                            const AtomicString& type,
                            const ErrorEventInit& initializer) {
    return new ErrorEvent(script_state, type, initializer);
  }
  static ErrorEvent* CreateSanitizedError(DOMWrapperWorld* world) {
    return new ErrorEvent("Script error.",
                          SourceLocation::Create(String(), 0, 0, nullptr),
                          ScriptValue(), world);
  }
  ~ErrorEvent() override;

  // As 'message' is exposed to JavaScript, never return unsanitizedMessage.
  const String& message() const { return sanitized_message_; }
  const String& filename() const { return location_->Url(); }
  unsigned lineno() const { return location_->LineNumber(); }
  unsigned colno() const { return location_->ColumnNumber(); }
  ScriptValue error(ScriptState*) const;

  // 'messageForConsole' is not exposed to JavaScript, and prefers
  // 'm_unsanitizedMessage'.
  const String& MessageForConsole() const {
    return !unsanitized_message_.IsEmpty() ? unsanitized_message_
                                           : sanitized_message_;
  }
  SourceLocation* Location() const { return location_.get(); }

  const AtomicString& InterfaceName() const override;

  DOMWrapperWorld* World() const { return world_.Get(); }

  void SetUnsanitizedMessage(const String&);

  DECLARE_VIRTUAL_TRACE();
  DECLARE_VIRTUAL_TRACE_WRAPPERS();

 private:
  ErrorEvent();
  ErrorEvent(const String& message,
             std::unique_ptr<SourceLocation>,
             ScriptValue error,
             DOMWrapperWorld*);
  ErrorEvent(ScriptState*, const AtomicString&, const ErrorEventInit&);

  String unsanitized_message_;
  String sanitized_message_;
  std::unique_ptr<SourceLocation> location_;
  TraceWrapperV8Reference<v8::Value> error_;

  RefPtr<DOMWrapperWorld> world_;
};

}  // namespace blink

#endif  // ErrorEvent_h

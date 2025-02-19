// Copyright (c) 2008, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "bindings/core/v8/custom/V8CustomXPathNSResolver.h"

#include "bindings/core/v8/ScriptController.h"
#include "bindings/core/v8/V8BindingForCore.h"
#include "core/dom/ExecutionContext.h"
#include "core/frame/FrameConsole.h"
#include "core/frame/LocalDOMWindow.h"
#include "core/frame/LocalFrame.h"
#include "core/inspector/ConsoleMessage.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

V8CustomXPathNSResolver* V8CustomXPathNSResolver::Create(
    ScriptState* script_state,
    v8::Local<v8::Object> resolver) {
  return new V8CustomXPathNSResolver(script_state, resolver);
}

V8CustomXPathNSResolver::V8CustomXPathNSResolver(ScriptState* script_state,
                                                 v8::Local<v8::Object> resolver)
    : script_state_(script_state), resolver_(resolver) {}

AtomicString V8CustomXPathNSResolver::lookupNamespaceURI(const String& prefix) {
  v8::Isolate* isolate = script_state_->GetIsolate();
  v8::Local<v8::Function> lookup_namespace_uri_func;
  v8::Local<v8::String> lookup_namespace_uri_name =
      V8AtomicString(isolate, "lookupNamespaceURI");

  // Check if the resolver has a function property named lookupNamespaceURI.
  v8::Local<v8::Value> lookup_namespace_uri;
  if (resolver_->Get(script_state_->GetContext(), lookup_namespace_uri_name)
          .ToLocal(&lookup_namespace_uri) &&
      lookup_namespace_uri->IsFunction())
    lookup_namespace_uri_func =
        v8::Local<v8::Function>::Cast(lookup_namespace_uri);

  if (lookup_namespace_uri_func.IsEmpty() && !resolver_->IsFunction()) {
    LocalFrame* frame = ToLocalFrameIfNotDetached(script_state_->GetContext());
    if (frame)
      frame->Console().AddMessage(ConsoleMessage::Create(
          kJSMessageSource, kErrorMessageLevel,
          "XPathNSResolver does not have a lookupNamespaceURI method."));
    return g_null_atom;
  }

  // Catch exceptions from calling the namespace resolver.
  v8::TryCatch try_catch(isolate);
  try_catch.SetVerbose(true);  // Print exceptions to console.

  const int kArgc = 1;
  v8::Local<v8::Value> argv[kArgc] = {V8String(isolate, prefix)};
  v8::Local<v8::Function> function =
      lookup_namespace_uri_func.IsEmpty()
          ? v8::Local<v8::Function>::Cast(resolver_)
          : lookup_namespace_uri_func;

  v8::Local<v8::Value> retval;
  // Eat exceptions from namespace resolver and return an empty string. This
  // will most likely cause NamespaceError.
  if (!V8ScriptRunner::CallFunction(
           function, ToExecutionContext(script_state_->GetContext()), resolver_,
           kArgc, argv, isolate)
           .ToLocal(&retval))
    return g_null_atom;

  TOSTRING_DEFAULT(V8StringResource<kTreatNullAsNullString>, return_string,
                   retval, g_null_atom);
  return return_string;
}

DEFINE_TRACE(V8CustomXPathNSResolver) {
  XPathNSResolver::Trace(visitor);
}

}  // namespace blink

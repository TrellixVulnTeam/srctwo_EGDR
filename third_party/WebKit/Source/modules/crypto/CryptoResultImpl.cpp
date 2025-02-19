/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
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

#include "modules/crypto/CryptoResultImpl.h"

#include "bindings/core/v8/Dictionary.h"
#include "bindings/core/v8/ScriptPromiseResolver.h"
#include "bindings/core/v8/V8ArrayBuffer.h"
#include "bindings/core/v8/V8BindingForCore.h"
#include "bindings/core/v8/V8ObjectBuilder.h"
#include "bindings/modules/v8/V8CryptoKey.h"
#include "core/dom/ContextLifecycleObserver.h"
#include "core/dom/DOMException.h"
#include "core/dom/ExecutionContext.h"
#include "core/typed_arrays/DOMArrayBuffer.h"
#include "modules/crypto/CryptoKey.h"
#include "modules/crypto/NormalizeAlgorithm.h"
#include "platform/bindings/ScriptState.h"
#include "platform/wtf/Atomics.h"
#include "public/platform/Platform.h"
#include "public/platform/WebCryptoAlgorithm.h"

namespace blink {

static void RejectWithTypeError(const String& error_details,
                                ScriptPromiseResolver* resolver) {
  // Duplicate some of the checks done by ScriptPromiseResolver.
  if (!resolver->GetExecutionContext() ||
      resolver->GetExecutionContext()->IsContextDestroyed())
    return;

  ScriptState::Scope scope(resolver->GetScriptState());
  v8::Isolate* isolate = resolver->GetScriptState()->GetIsolate();
  resolver->Reject(v8::Exception::TypeError(V8String(isolate, error_details)));
}

class CryptoResultImpl::Resolver final : public ScriptPromiseResolver {
 public:
  static Resolver* Create(ScriptState* script_state, CryptoResultImpl* result) {
    DCHECK(script_state->ContextIsValid());
    Resolver* resolver = new Resolver(script_state, result);
    resolver->SuspendIfNeeded();
    resolver->KeepAliveWhilePending();
    return resolver;
  }

  void ContextDestroyed(ExecutionContext* destroyed_context) override {
    result_->Cancel();
    result_ = nullptr;
    ScriptPromiseResolver::ContextDestroyed(destroyed_context);
  }

  DEFINE_INLINE_VIRTUAL_TRACE() {
    visitor->Trace(result_);
    ScriptPromiseResolver::Trace(visitor);
  }

 private:
  Resolver(ScriptState* script_state, CryptoResultImpl* result)
      : ScriptPromiseResolver(script_state), result_(result) {}

  Member<CryptoResultImpl> result_;
};

CryptoResultImpl::ResultCancel::ResultCancel() : cancelled_(0) {}

bool CryptoResultImpl::ResultCancel::Cancelled() const {
  return AcquireLoad(&cancelled_);
}

void CryptoResultImpl::ResultCancel::Cancel() {
  ReleaseStore(&cancelled_, 1);
}

ExceptionCode WebCryptoErrorToExceptionCode(WebCryptoErrorType error_type) {
  switch (error_type) {
    case kWebCryptoErrorTypeNotSupported:
      return kNotSupportedError;
    case kWebCryptoErrorTypeSyntax:
      return kSyntaxError;
    case kWebCryptoErrorTypeInvalidAccess:
      return kInvalidAccessError;
    case kWebCryptoErrorTypeData:
      return kDataError;
    case kWebCryptoErrorTypeOperation:
      return kOperationError;
    case kWebCryptoErrorTypeType:
      return kV8TypeError;
  }

  NOTREACHED();
  return 0;
}

CryptoResultImpl::CryptoResultImpl(ScriptState* script_state)
    : resolver_(Resolver::Create(script_state, this)),
      cancel_(ResultCancel::Create()) {
  // Sync cancellation state.
  if (ExecutionContext::From(script_state)->IsContextDestroyed())
    cancel_->Cancel();
}

CryptoResultImpl::~CryptoResultImpl() {
  DCHECK(!resolver_);
}

DEFINE_TRACE(CryptoResultImpl) {
  visitor->Trace(resolver_);
  CryptoResult::Trace(visitor);
}

void CryptoResultImpl::ClearResolver() {
  resolver_ = nullptr;
}

CryptoResultImpl* CryptoResultImpl::Create(ScriptState* script_state) {
  return new CryptoResultImpl(script_state);
}

void CryptoResultImpl::CompleteWithError(WebCryptoErrorType error_type,
                                         const WebString& error_details) {
  if (!resolver_)
    return;

  ExceptionCode ec = WebCryptoErrorToExceptionCode(error_type);

  // Handle TypeError separately, as it cannot be created using
  // DOMException.
  if (ec == kV8TypeError)
    RejectWithTypeError(error_details, resolver_);
  else
    resolver_->Reject(DOMException::Create(ec, error_details));
  ClearResolver();
}

void CryptoResultImpl::CompleteWithBuffer(const void* bytes,
                                          unsigned bytes_size) {
  if (!resolver_)
    return;

  resolver_->Resolve(DOMArrayBuffer::Create(bytes, bytes_size));
  ClearResolver();
}

void CryptoResultImpl::CompleteWithJson(const char* utf8_data,
                                        unsigned length) {
  if (!resolver_)
    return;

  ScriptState* script_state = resolver_->GetScriptState();
  ScriptState::Scope scope(script_state);

  v8::Local<v8::String> json_string =
      V8StringFromUtf8(script_state->GetIsolate(), utf8_data, length);

  v8::TryCatch exception_catcher(script_state->GetIsolate());
  v8::Local<v8::Value> json_dictionary;
  if (v8::JSON::Parse(script_state->GetIsolate(), json_string)
          .ToLocal(&json_dictionary))
    resolver_->Resolve(json_dictionary);
  else
    resolver_->Reject(exception_catcher.Exception());
  ClearResolver();
}

void CryptoResultImpl::CompleteWithBoolean(bool b) {
  if (!resolver_)
    return;

  resolver_->Resolve(b);
  ClearResolver();
}

void CryptoResultImpl::CompleteWithKey(const WebCryptoKey& key) {
  if (!resolver_)
    return;

  resolver_->Resolve(CryptoKey::Create(key));
  ClearResolver();
}

void CryptoResultImpl::CompleteWithKeyPair(const WebCryptoKey& public_key,
                                           const WebCryptoKey& private_key) {
  if (!resolver_)
    return;

  ScriptState* script_state = resolver_->GetScriptState();
  ScriptState::Scope scope(script_state);

  V8ObjectBuilder key_pair(script_state);

  key_pair.Add("publicKey",
               ScriptValue::From(script_state, CryptoKey::Create(public_key)));
  key_pair.Add("privateKey",
               ScriptValue::From(script_state, CryptoKey::Create(private_key)));

  resolver_->Resolve(key_pair.V8Value());
  ClearResolver();
}

void CryptoResultImpl::Cancel() {
  DCHECK(cancel_);
  cancel_->Cancel();
  cancel_.Clear();
  ClearResolver();
}

ScriptPromise CryptoResultImpl::Promise() {
  return resolver_ ? resolver_->Promise() : ScriptPromise();
}

}  // namespace blink

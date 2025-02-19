// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file has been auto-generated by code_generator_v8.py.
// DO NOT MODIFY!

// This file has been generated from the Jinja2 template in
// third_party/WebKit/Source/bindings/templates/callback_function.h.tmpl

// clang-format off

#ifndef V8AnyCallbackFunctionOptionalAnyArg_h
#define V8AnyCallbackFunctionOptionalAnyArg_h

#include "bindings/core/v8/NativeValueTraits.h"
#include "core/CoreExport.h"
#include "platform/bindings/ScriptWrappable.h"
#include "platform/bindings/TraceWrapperV8Reference.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class ScriptState;

class CORE_EXPORT V8AnyCallbackFunctionOptionalAnyArg final : public GarbageCollectedFinalized<V8AnyCallbackFunctionOptionalAnyArg>, public TraceWrapperBase {
 public:
  static V8AnyCallbackFunctionOptionalAnyArg* Create(ScriptState*, v8::Local<v8::Value> callback);

  ~V8AnyCallbackFunctionOptionalAnyArg() = default;

  DEFINE_INLINE_TRACE() {}
  DECLARE_TRACE_WRAPPERS();

  bool call(ScriptWrappable* scriptWrappable, ScriptValue optionalAnyArg, ScriptValue& returnValue);

  v8::Local<v8::Function> v8Value(v8::Isolate* isolate) {
    return callback_.NewLocal(isolate);
  }

 private:
  V8AnyCallbackFunctionOptionalAnyArg(ScriptState*, v8::Local<v8::Function>);

  RefPtr<ScriptState> script_state_;
  TraceWrapperV8Reference<v8::Function> callback_;
};

template <>
struct NativeValueTraits<V8AnyCallbackFunctionOptionalAnyArg> : public NativeValueTraitsBase<V8AnyCallbackFunctionOptionalAnyArg> {
  CORE_EXPORT static V8AnyCallbackFunctionOptionalAnyArg* NativeValue(v8::Isolate*, v8::Local<v8::Value>, ExceptionState&);
};

}  // namespace blink

#endif  // V8AnyCallbackFunctionOptionalAnyArg_h

// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bindings/core/v8/V8IteratorResultValue.h"

namespace blink {

v8::Local<v8::Object> V8IteratorResultValue(v8::Isolate* isolate,
                                            bool done,
                                            v8::Local<v8::Value> value) {
  v8::Local<v8::Object> result = v8::Object::New(isolate);
  if (value.IsEmpty())
    value = v8::Undefined(isolate);
  if (!V8CallBoolean(result->CreateDataProperty(
          isolate->GetCurrentContext(), V8AtomicString(isolate, "done"),
          v8::Boolean::New(isolate, done))) ||
      !V8CallBoolean(
          result->CreateDataProperty(isolate->GetCurrentContext(),
                                     V8AtomicString(isolate, "value"), value)))
    return v8::Local<v8::Object>();
  return result;
}

v8::MaybeLocal<v8::Value> V8UnpackIteratorResult(ScriptState* script_state,
                                                 v8::Local<v8::Object> result,
                                                 bool* done) {
  v8::MaybeLocal<v8::Value> maybe_value =
      result->Get(script_state->GetContext(),
                  V8AtomicString(script_state->GetIsolate(), "value"));
  if (maybe_value.IsEmpty())
    return maybe_value;
  v8::Local<v8::Value> done_value;
  if (!result
           ->Get(script_state->GetContext(),
                 V8AtomicString(script_state->GetIsolate(), "done"))
           .ToLocal(&done_value) ||
      !done_value->BooleanValue(script_state->GetContext()).To(done)) {
    return v8::MaybeLocal<v8::Value>();
  }
  return maybe_value;
}

}  // namespace blink

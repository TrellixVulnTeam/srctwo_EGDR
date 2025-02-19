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

#ifndef V8ObjectConstructor_h
#define V8ObjectConstructor_h

#include "platform/PlatformExport.h"
#include "platform/bindings/V8PerIsolateData.h"
#include "platform/wtf/Allocator.h"
#include "v8/include/v8.h"

namespace blink {

class ConstructorMode {
  STACK_ALLOCATED();

 public:
  enum Mode { kWrapExistingObject, kCreateNewObject };

  ConstructorMode(v8::Isolate* isolate) : isolate_(isolate) {
    V8PerIsolateData* data = V8PerIsolateData::From(isolate_);
    previous_ = data->constructor_mode_;
    data->constructor_mode_ = kWrapExistingObject;
  }

  ~ConstructorMode() {
    V8PerIsolateData* data = V8PerIsolateData::From(isolate_);
    data->constructor_mode_ = previous_;
  }

  static bool Current(v8::Isolate* isolate) {
    return V8PerIsolateData::From(isolate)->constructor_mode_;
  }

 private:
  v8::Isolate* isolate_;
  bool previous_;
};

class PLATFORM_EXPORT V8ObjectConstructor {
  STATIC_ONLY(V8ObjectConstructor);

 public:
  static v8::MaybeLocal<v8::Object> NewInstance(
      v8::Isolate*,
      v8::Local<v8::Function>,
      int argc = 0,
      v8::Local<v8::Value> argv[] = nullptr);

  static void IsValidConstructorMode(
      const v8::FunctionCallbackInfo<v8::Value>&);
};

}  // namespace blink

#endif  // V8ObjectConstructor_h

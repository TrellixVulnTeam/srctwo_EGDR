// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file has been auto-generated by code_generator_v8.py.
// DO NOT MODIFY!

// This file has been generated from the Jinja2 template in
// third_party/WebKit/Source/bindings/templates/union_container.cpp.tmpl

// clang-format off
#include "unsigned_long_long_or_boolean_or_test_callback_interface.h"

#include "bindings/core/v8/IDLTypes.h"
#include "bindings/core/v8/NativeValueTraitsImpl.h"
#include "bindings/core/v8/ToV8ForCore.h"
#include "bindings/core/v8/V8TestCallbackInterface.h"

namespace blink {

UnsignedLongLongOrBooleanOrTestCallbackInterface::UnsignedLongLongOrBooleanOrTestCallbackInterface() : type_(SpecificTypeNone) {}

bool UnsignedLongLongOrBooleanOrTestCallbackInterface::getAsBoolean() const {
  DCHECK(isBoolean());
  return boolean_;
}

void UnsignedLongLongOrBooleanOrTestCallbackInterface::setBoolean(bool value) {
  DCHECK(isNull());
  boolean_ = value;
  type_ = SpecificTypeBoolean;
}

UnsignedLongLongOrBooleanOrTestCallbackInterface UnsignedLongLongOrBooleanOrTestCallbackInterface::fromBoolean(bool value) {
  UnsignedLongLongOrBooleanOrTestCallbackInterface container;
  container.setBoolean(value);
  return container;
}

TestCallbackInterface* UnsignedLongLongOrBooleanOrTestCallbackInterface::getAsTestCallbackInterface() const {
  DCHECK(isTestCallbackInterface());
  return test_callback_interface_;
}

void UnsignedLongLongOrBooleanOrTestCallbackInterface::setTestCallbackInterface(TestCallbackInterface* value) {
  DCHECK(isNull());
  test_callback_interface_ = value;
  type_ = SpecificTypeTestCallbackInterface;
}

UnsignedLongLongOrBooleanOrTestCallbackInterface UnsignedLongLongOrBooleanOrTestCallbackInterface::fromTestCallbackInterface(TestCallbackInterface* value) {
  UnsignedLongLongOrBooleanOrTestCallbackInterface container;
  container.setTestCallbackInterface(value);
  return container;
}

uint64_t UnsignedLongLongOrBooleanOrTestCallbackInterface::getAsUnsignedLongLong() const {
  DCHECK(isUnsignedLongLong());
  return unsigned_long_long_;
}

void UnsignedLongLongOrBooleanOrTestCallbackInterface::setUnsignedLongLong(uint64_t value) {
  DCHECK(isNull());
  unsigned_long_long_ = value;
  type_ = SpecificTypeUnsignedLongLong;
}

UnsignedLongLongOrBooleanOrTestCallbackInterface UnsignedLongLongOrBooleanOrTestCallbackInterface::fromUnsignedLongLong(uint64_t value) {
  UnsignedLongLongOrBooleanOrTestCallbackInterface container;
  container.setUnsignedLongLong(value);
  return container;
}

UnsignedLongLongOrBooleanOrTestCallbackInterface::UnsignedLongLongOrBooleanOrTestCallbackInterface(const UnsignedLongLongOrBooleanOrTestCallbackInterface&) = default;
UnsignedLongLongOrBooleanOrTestCallbackInterface::~UnsignedLongLongOrBooleanOrTestCallbackInterface() = default;
UnsignedLongLongOrBooleanOrTestCallbackInterface& UnsignedLongLongOrBooleanOrTestCallbackInterface::operator=(const UnsignedLongLongOrBooleanOrTestCallbackInterface&) = default;

DEFINE_TRACE(UnsignedLongLongOrBooleanOrTestCallbackInterface) {
  visitor->Trace(test_callback_interface_);
}

void V8UnsignedLongLongOrBooleanOrTestCallbackInterface::ToImpl(v8::Isolate* isolate, v8::Local<v8::Value> v8Value, UnsignedLongLongOrBooleanOrTestCallbackInterface& impl, UnionTypeConversionMode conversionMode, ExceptionState& exceptionState) {
  if (v8Value.IsEmpty())
    return;

  if (conversionMode == UnionTypeConversionMode::kNullable && IsUndefinedOrNull(v8Value))
    return;

  if (V8TestCallbackInterface::hasInstance(v8Value, isolate)) {
    TestCallbackInterface* cppValue = V8TestCallbackInterface::ToImpl(v8::Local<v8::Object>::Cast(v8Value));
    impl.setTestCallbackInterface(cppValue);
    return;
  }

  if (v8Value->IsBoolean()) {
    impl.setBoolean(v8Value.As<v8::Boolean>()->Value());
    return;
  }

  if (v8Value->IsNumber()) {
    uint64_t cppValue = NativeValueTraits<IDLUnsignedLongLong>::NativeValue(isolate, v8Value, exceptionState, kNormalConversion);
    if (exceptionState.HadException())
      return;
    impl.setUnsignedLongLong(cppValue);
    return;
  }

  {
    uint64_t cppValue = NativeValueTraits<IDLUnsignedLongLong>::NativeValue(isolate, v8Value, exceptionState, kNormalConversion);
    if (exceptionState.HadException())
      return;
    impl.setUnsignedLongLong(cppValue);
    return;
  }
}

v8::Local<v8::Value> ToV8(const UnsignedLongLongOrBooleanOrTestCallbackInterface& impl, v8::Local<v8::Object> creationContext, v8::Isolate* isolate) {
  switch (impl.type_) {
    case UnsignedLongLongOrBooleanOrTestCallbackInterface::SpecificTypeNone:
      return v8::Null(isolate);
    case UnsignedLongLongOrBooleanOrTestCallbackInterface::SpecificTypeBoolean:
      return v8::Boolean::New(isolate, impl.getAsBoolean());
    case UnsignedLongLongOrBooleanOrTestCallbackInterface::SpecificTypeTestCallbackInterface:
      return ToV8(impl.getAsTestCallbackInterface(), creationContext, isolate);
    case UnsignedLongLongOrBooleanOrTestCallbackInterface::SpecificTypeUnsignedLongLong:
      return v8::Number::New(isolate, static_cast<double>(impl.getAsUnsignedLongLong()));
    default:
      NOTREACHED();
  }
  return v8::Local<v8::Value>();
}

UnsignedLongLongOrBooleanOrTestCallbackInterface NativeValueTraits<UnsignedLongLongOrBooleanOrTestCallbackInterface>::NativeValue(v8::Isolate* isolate, v8::Local<v8::Value> value, ExceptionState& exceptionState) {
  UnsignedLongLongOrBooleanOrTestCallbackInterface impl;
  V8UnsignedLongLongOrBooleanOrTestCallbackInterface::ToImpl(isolate, value, impl, UnionTypeConversionMode::kNotNullable, exceptionState);
  return impl;
}

}  // namespace blink

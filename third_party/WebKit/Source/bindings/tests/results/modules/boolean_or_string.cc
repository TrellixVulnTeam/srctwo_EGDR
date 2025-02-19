// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file has been auto-generated by code_generator_v8.py.
// DO NOT MODIFY!

// This file has been generated from the Jinja2 template in
// third_party/WebKit/Source/bindings/templates/union_container.cpp.tmpl

// clang-format off
#include "boolean_or_string.h"

#include "bindings/core/v8/IDLTypes.h"
#include "bindings/core/v8/NativeValueTraitsImpl.h"
#include "bindings/core/v8/ToV8ForCore.h"

namespace blink {

BooleanOrString::BooleanOrString() : type_(SpecificTypeNone) {}

bool BooleanOrString::getAsBoolean() const {
  DCHECK(isBoolean());
  return boolean_;
}

void BooleanOrString::setBoolean(bool value) {
  DCHECK(isNull());
  boolean_ = value;
  type_ = SpecificTypeBoolean;
}

BooleanOrString BooleanOrString::fromBoolean(bool value) {
  BooleanOrString container;
  container.setBoolean(value);
  return container;
}

const String& BooleanOrString::getAsString() const {
  DCHECK(isString());
  return string_;
}

void BooleanOrString::setString(const String& value) {
  DCHECK(isNull());
  string_ = value;
  type_ = SpecificTypeString;
}

BooleanOrString BooleanOrString::fromString(const String& value) {
  BooleanOrString container;
  container.setString(value);
  return container;
}

BooleanOrString::BooleanOrString(const BooleanOrString&) = default;
BooleanOrString::~BooleanOrString() = default;
BooleanOrString& BooleanOrString::operator=(const BooleanOrString&) = default;

DEFINE_TRACE(BooleanOrString) {
}

void V8BooleanOrString::ToImpl(v8::Isolate* isolate, v8::Local<v8::Value> v8Value, BooleanOrString& impl, UnionTypeConversionMode conversionMode, ExceptionState& exceptionState) {
  if (v8Value.IsEmpty())
    return;

  if (conversionMode == UnionTypeConversionMode::kNullable && IsUndefinedOrNull(v8Value))
    return;

  if (v8Value->IsBoolean()) {
    impl.setBoolean(v8Value.As<v8::Boolean>()->Value());
    return;
  }

  {
    V8StringResource<> cppValue = v8Value;
    if (!cppValue.Prepare(exceptionState))
      return;
    impl.setString(cppValue);
    return;
  }
}

v8::Local<v8::Value> ToV8(const BooleanOrString& impl, v8::Local<v8::Object> creationContext, v8::Isolate* isolate) {
  switch (impl.type_) {
    case BooleanOrString::SpecificTypeNone:
      return v8::Null(isolate);
    case BooleanOrString::SpecificTypeBoolean:
      return v8::Boolean::New(isolate, impl.getAsBoolean());
    case BooleanOrString::SpecificTypeString:
      return V8String(isolate, impl.getAsString());
    default:
      NOTREACHED();
  }
  return v8::Local<v8::Value>();
}

BooleanOrString NativeValueTraits<BooleanOrString>::NativeValue(v8::Isolate* isolate, v8::Local<v8::Value> value, ExceptionState& exceptionState) {
  BooleanOrString impl;
  V8BooleanOrString::ToImpl(isolate, value, impl, UnionTypeConversionMode::kNotNullable, exceptionState);
  return impl;
}

}  // namespace blink

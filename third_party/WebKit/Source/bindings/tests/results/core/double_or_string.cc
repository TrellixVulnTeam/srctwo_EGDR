// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file has been auto-generated by code_generator_v8.py.
// DO NOT MODIFY!

// This file has been generated from the Jinja2 template in
// third_party/WebKit/Source/bindings/templates/union_container.cpp.tmpl

// clang-format off
#include "double_or_string.h"

#include "bindings/core/v8/IDLTypes.h"
#include "bindings/core/v8/NativeValueTraitsImpl.h"
#include "bindings/core/v8/ToV8ForCore.h"

namespace blink {

DoubleOrString::DoubleOrString() : type_(SpecificTypeNone) {}

double DoubleOrString::getAsDouble() const {
  DCHECK(isDouble());
  return double_;
}

void DoubleOrString::setDouble(double value) {
  DCHECK(isNull());
  double_ = value;
  type_ = SpecificTypeDouble;
}

DoubleOrString DoubleOrString::fromDouble(double value) {
  DoubleOrString container;
  container.setDouble(value);
  return container;
}

const String& DoubleOrString::getAsString() const {
  DCHECK(isString());
  return string_;
}

void DoubleOrString::setString(const String& value) {
  DCHECK(isNull());
  string_ = value;
  type_ = SpecificTypeString;
}

DoubleOrString DoubleOrString::fromString(const String& value) {
  DoubleOrString container;
  container.setString(value);
  return container;
}

DoubleOrString::DoubleOrString(const DoubleOrString&) = default;
DoubleOrString::~DoubleOrString() = default;
DoubleOrString& DoubleOrString::operator=(const DoubleOrString&) = default;

DEFINE_TRACE(DoubleOrString) {
}

void V8DoubleOrString::ToImpl(v8::Isolate* isolate, v8::Local<v8::Value> v8Value, DoubleOrString& impl, UnionTypeConversionMode conversionMode, ExceptionState& exceptionState) {
  if (v8Value.IsEmpty())
    return;

  if (conversionMode == UnionTypeConversionMode::kNullable && IsUndefinedOrNull(v8Value))
    return;

  if (v8Value->IsNumber()) {
    double cppValue = NativeValueTraits<IDLDouble>::NativeValue(isolate, v8Value, exceptionState);
    if (exceptionState.HadException())
      return;
    impl.setDouble(cppValue);
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

v8::Local<v8::Value> ToV8(const DoubleOrString& impl, v8::Local<v8::Object> creationContext, v8::Isolate* isolate) {
  switch (impl.type_) {
    case DoubleOrString::SpecificTypeNone:
      return v8::Null(isolate);
    case DoubleOrString::SpecificTypeDouble:
      return v8::Number::New(isolate, impl.getAsDouble());
    case DoubleOrString::SpecificTypeString:
      return V8String(isolate, impl.getAsString());
    default:
      NOTREACHED();
  }
  return v8::Local<v8::Value>();
}

DoubleOrString NativeValueTraits<DoubleOrString>::NativeValue(v8::Isolate* isolate, v8::Local<v8::Value> value, ExceptionState& exceptionState) {
  DoubleOrString impl;
  V8DoubleOrString::ToImpl(isolate, value, impl, UnionTypeConversionMode::kNotNullable, exceptionState);
  return impl;
}

}  // namespace blink

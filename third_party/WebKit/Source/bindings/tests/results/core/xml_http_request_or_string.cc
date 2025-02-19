// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file has been auto-generated by code_generator_v8.py.
// DO NOT MODIFY!

// This file has been generated from the Jinja2 template in
// third_party/WebKit/Source/bindings/templates/union_container.cpp.tmpl

// clang-format off
#include "xml_http_request_or_string.h"

#include "bindings/core/v8/IDLTypes.h"
#include "bindings/core/v8/NativeValueTraitsImpl.h"
#include "bindings/core/v8/ToV8ForCore.h"
#include "bindings/core/v8/V8XMLHttpRequest.h"

namespace blink {

XMLHttpRequestOrString::XMLHttpRequestOrString() : type_(SpecificTypeNone) {}

const String& XMLHttpRequestOrString::getAsString() const {
  DCHECK(isString());
  return string_;
}

void XMLHttpRequestOrString::setString(const String& value) {
  DCHECK(isNull());
  string_ = value;
  type_ = SpecificTypeString;
}

XMLHttpRequestOrString XMLHttpRequestOrString::fromString(const String& value) {
  XMLHttpRequestOrString container;
  container.setString(value);
  return container;
}

XMLHttpRequest* XMLHttpRequestOrString::getAsXMLHttpRequest() const {
  DCHECK(isXMLHttpRequest());
  return xml_http_request_;
}

void XMLHttpRequestOrString::setXMLHttpRequest(XMLHttpRequest* value) {
  DCHECK(isNull());
  xml_http_request_ = value;
  type_ = SpecificTypeXMLHttpRequest;
}

XMLHttpRequestOrString XMLHttpRequestOrString::fromXMLHttpRequest(XMLHttpRequest* value) {
  XMLHttpRequestOrString container;
  container.setXMLHttpRequest(value);
  return container;
}

XMLHttpRequestOrString::XMLHttpRequestOrString(const XMLHttpRequestOrString&) = default;
XMLHttpRequestOrString::~XMLHttpRequestOrString() = default;
XMLHttpRequestOrString& XMLHttpRequestOrString::operator=(const XMLHttpRequestOrString&) = default;

DEFINE_TRACE(XMLHttpRequestOrString) {
  visitor->Trace(xml_http_request_);
}

void V8XMLHttpRequestOrString::ToImpl(v8::Isolate* isolate, v8::Local<v8::Value> v8Value, XMLHttpRequestOrString& impl, UnionTypeConversionMode conversionMode, ExceptionState& exceptionState) {
  if (v8Value.IsEmpty())
    return;

  if (conversionMode == UnionTypeConversionMode::kNullable && IsUndefinedOrNull(v8Value))
    return;

  if (V8XMLHttpRequest::hasInstance(v8Value, isolate)) {
    XMLHttpRequest* cppValue = V8XMLHttpRequest::ToImpl(v8::Local<v8::Object>::Cast(v8Value));
    impl.setXMLHttpRequest(cppValue);
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

v8::Local<v8::Value> ToV8(const XMLHttpRequestOrString& impl, v8::Local<v8::Object> creationContext, v8::Isolate* isolate) {
  switch (impl.type_) {
    case XMLHttpRequestOrString::SpecificTypeNone:
      return v8::Null(isolate);
    case XMLHttpRequestOrString::SpecificTypeString:
      return V8String(isolate, impl.getAsString());
    case XMLHttpRequestOrString::SpecificTypeXMLHttpRequest:
      return ToV8(impl.getAsXMLHttpRequest(), creationContext, isolate);
    default:
      NOTREACHED();
  }
  return v8::Local<v8::Value>();
}

XMLHttpRequestOrString NativeValueTraits<XMLHttpRequestOrString>::NativeValue(v8::Isolate* isolate, v8::Local<v8::Value> value, ExceptionState& exceptionState) {
  XMLHttpRequestOrString impl;
  V8XMLHttpRequestOrString::ToImpl(isolate, value, impl, UnionTypeConversionMode::kNotNullable, exceptionState);
  return impl;
}

}  // namespace blink

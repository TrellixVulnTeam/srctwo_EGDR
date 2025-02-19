// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file has been auto-generated by code_generator_v8.py.
// DO NOT MODIFY!

// This file has been generated from the Jinja2 template in
// third_party/WebKit/Source/bindings/templates/interface.cpp.tmpl

// clang-format off
#include "V8ArrayBufferView.h"

#include "bindings/core/v8/ExceptionState.h"
#include "bindings/core/v8/IDLTypes.h"
#include "bindings/core/v8/NativeValueTraitsImpl.h"
#include "bindings/core/v8/V8ArrayBuffer.h"
#include "bindings/core/v8/V8DOMConfiguration.h"
#include "bindings/core/v8/V8DataView.h"
#include "bindings/core/v8/V8Float32Array.h"
#include "bindings/core/v8/V8Float64Array.h"
#include "bindings/core/v8/V8Int16Array.h"
#include "bindings/core/v8/V8Int32Array.h"
#include "bindings/core/v8/V8Int8Array.h"
#include "bindings/core/v8/V8SharedArrayBuffer.h"
#include "bindings/core/v8/V8Uint16Array.h"
#include "bindings/core/v8/V8Uint32Array.h"
#include "bindings/core/v8/V8Uint8Array.h"
#include "bindings/core/v8/V8Uint8ClampedArray.h"
#include "core/dom/ExecutionContext.h"
#include "platform/bindings/RuntimeCallStats.h"
#include "platform/bindings/V8ObjectConstructor.h"
#include "platform/bindings/V8PrivateProperty.h"
#include "platform/wtf/GetPtr.h"
#include "platform/wtf/RefPtr.h"

namespace blink {

// Suppress warning: global constructors, because struct WrapperTypeInfo is trivial
// and does not depend on another global objects.
#if defined(COMPONENT_BUILD) && defined(WIN32) && defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
const WrapperTypeInfo V8ArrayBufferView::wrapperTypeInfo = {
    gin::kEmbedderBlink,
    nullptr,
    V8ArrayBufferView::Trace,
    V8ArrayBufferView::TraceWrappers,
    nullptr,
    "ArrayBufferView",
    nullptr,
    WrapperTypeInfo::kWrapperTypeObjectPrototype,
    WrapperTypeInfo::kObjectClassId,
    WrapperTypeInfo::kNotInheritFromActiveScriptWrappable,
    WrapperTypeInfo::kIndependent,
};
#if defined(COMPONENT_BUILD) && defined(WIN32) && defined(__clang__)
#pragma clang diagnostic pop
#endif

// This static member must be declared by DEFINE_WRAPPERTYPEINFO in TestArrayBufferView.h.
// For details, see the comment of DEFINE_WRAPPERTYPEINFO in
// platform/bindings/ScriptWrappable.h.
const WrapperTypeInfo& TestArrayBufferView::wrapper_type_info_ = V8ArrayBufferView::wrapperTypeInfo;

// not [ActiveScriptWrappable]
static_assert(
    !std::is_base_of<ActiveScriptWrappableBase, TestArrayBufferView>::value,
    "TestArrayBufferView inherits from ActiveScriptWrappable<>, but is not specifying "
    "[ActiveScriptWrappable] extended attribute in the IDL file.  "
    "Be consistent.");
static_assert(
    std::is_same<decltype(&TestArrayBufferView::HasPendingActivity),
                 decltype(&ScriptWrappable::HasPendingActivity)>::value,
    "TestArrayBufferView is overriding hasPendingActivity(), but is not specifying "
    "[ActiveScriptWrappable] extended attribute in the IDL file.  "
    "Be consistent.");

TestArrayBufferView* V8ArrayBufferView::ToImpl(v8::Local<v8::Object> object) {
  DCHECK(object->IsArrayBufferView());
  ScriptWrappable* scriptWrappable = ToScriptWrappable(object);
  if (scriptWrappable)
    return scriptWrappable->ToImpl<TestArrayBufferView>();

  if (object->IsInt8Array())
    return V8Int8Array::ToImpl(object);
  if (object->IsInt16Array())
    return V8Int16Array::ToImpl(object);
  if (object->IsInt32Array())
    return V8Int32Array::ToImpl(object);
  if (object->IsUint8Array())
    return V8Uint8Array::ToImpl(object);
  if (object->IsUint8ClampedArray())
    return V8Uint8ClampedArray::ToImpl(object);
  if (object->IsUint16Array())
    return V8Uint16Array::ToImpl(object);
  if (object->IsUint32Array())
    return V8Uint32Array::ToImpl(object);
  if (object->IsFloat32Array())
    return V8Float32Array::ToImpl(object);
  if (object->IsFloat64Array())
    return V8Float64Array::ToImpl(object);
  if (object->IsDataView())
    return V8DataView::ToImpl(object);

  NOTREACHED();
  return 0;
}

TestArrayBufferView* V8ArrayBufferView::ToImplWithTypeCheck(v8::Isolate* isolate, v8::Local<v8::Value> value) {
  return value->IsArrayBufferView() ? ToImpl(v8::Local<v8::Object>::Cast(value)) : nullptr;
}

TestArrayBufferView* NativeValueTraits<TestArrayBufferView>::NativeValue(v8::Isolate* isolate, v8::Local<v8::Value> value, ExceptionState& exceptionState) {
  TestArrayBufferView* nativeValue = V8ArrayBufferView::ToImplWithTypeCheck(isolate, value);
  if (!nativeValue) {
    exceptionState.ThrowTypeError(ExceptionMessages::FailedToConvertJSValue(
        "ArrayBufferView"));
  }
  return nativeValue;
}

}  // namespace blink

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CSSPaintDefinition_h
#define CSSPaintDefinition_h

#include "core/CSSPropertyNames.h"
#include "core/css/CSSSyntaxDescriptor.h"
#include "core/css/cssom/CSSStyleValue.h"
#include "platform/bindings/ScriptWrappable.h"
#include "platform/bindings/TraceWrapperMember.h"
#include "platform/bindings/TraceWrapperV8Reference.h"
#include "platform/geometry/IntSize.h"
#include "platform/heap/Handle.h"
#include "v8/include/v8.h"

namespace blink {

class Image;
class ScriptState;
class ImageResourceObserver;

// Represents a javascript class registered on the PaintWorkletGlobalScope by
// the author. It will store the properties for invalidation and input argument
// types as well.
class CSSPaintDefinition final
    : public GarbageCollectedFinalized<CSSPaintDefinition>,
      public TraceWrapperBase {
 public:
  static CSSPaintDefinition* Create(
      ScriptState*,
      v8::Local<v8::Function> constructor,
      v8::Local<v8::Function> paint,
      const Vector<CSSPropertyID>&,
      const Vector<AtomicString>& custom_invalidation_properties,
      const Vector<CSSSyntaxDescriptor>& input_argument_types,
      bool has_alpha);
  virtual ~CSSPaintDefinition();

  // Invokes the javascript 'paint' callback on an instance of the javascript
  // class. The size given will be the size of the PaintRenderingContext2D
  // given to the callback.
  //
  // This may return a nullptr (representing an invalid image) if javascript
  // throws an error.
  PassRefPtr<Image> Paint(const ImageResourceObserver&,
                          const IntSize&,
                          const CSSStyleValueVector*);
  const Vector<CSSPropertyID>& NativeInvalidationProperties() const {
    return native_invalidation_properties_;
  }
  const Vector<AtomicString>& CustomInvalidationProperties() const {
    return custom_invalidation_properties_;
  }
  const Vector<CSSSyntaxDescriptor>& InputArgumentTypes() const {
    return input_argument_types_;
  }
  bool HasAlpha() const { return has_alpha_; }

  ScriptState* GetScriptState() const { return script_state_.Get(); }

  v8::Local<v8::Function> PaintFunctionForTesting(v8::Isolate* isolate) {
    return paint_.NewLocal(isolate);
  }

  DEFINE_INLINE_TRACE(){};
  DECLARE_TRACE_WRAPPERS();

 private:
  CSSPaintDefinition(
      ScriptState*,
      v8::Local<v8::Function> constructor,
      v8::Local<v8::Function> paint,
      const Vector<CSSPropertyID>& native_invalidation_properties,
      const Vector<AtomicString>& custom_invalidation_properties,
      const Vector<CSSSyntaxDescriptor>& input_argument_types,
      bool has_alpha);

  void MaybeCreatePaintInstance();

  RefPtr<ScriptState> script_state_;

  // This object keeps the class instance object, constructor function and
  // paint function alive. It participates in wrapper tracing as it holds onto
  // V8 wrappers.
  TraceWrapperV8Reference<v8::Function> constructor_;
  TraceWrapperV8Reference<v8::Function> paint_;

  // At the moment there is only ever one instance of a paint class per type.
  TraceWrapperV8Reference<v8::Object> instance_;

  bool did_call_constructor_;

  Vector<CSSPropertyID> native_invalidation_properties_;
  Vector<AtomicString> custom_invalidation_properties_;
  // Input argument types, if applicable.
  Vector<CSSSyntaxDescriptor> input_argument_types_;
  bool has_alpha_;
};

}  // namespace blink

#endif  // CSSPaintDefinition_h

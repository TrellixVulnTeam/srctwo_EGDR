/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef Dictionary_h
#define Dictionary_h

#include "bindings/core/v8/DictionaryIterator.h"
#include "bindings/core/v8/Nullable.h"
#include "bindings/core/v8/V8BindingForCore.h"
#include "core/CoreExport.h"
#include "platform/wtf/HashMap.h"
#include "platform/wtf/Vector.h"
#include "platform/wtf/text/StringView.h"
#include "v8/include/v8.h"

namespace blink {

class ExceptionState;
class ExecutionContext;

// Dictionary class provides ways to retrieve property values as C++ objects
// from a V8 object. Instances of this class must not outlive V8's handle scope
// because they hold a V8 value without putting it on persistent handles.
class CORE_EXPORT Dictionary final {
  DISALLOW_NEW_EXCEPT_PLACEMENT_NEW();

 public:
  Dictionary() : isolate_(nullptr) {}
  Dictionary(v8::Isolate*,
             v8::Local<v8::Value> dictionary_object,
             ExceptionState&);

  Dictionary& operator=(const Dictionary&) = default;

  bool IsObject() const { return !dictionary_object_.IsEmpty(); }
  bool IsUndefinedOrNull() const { return !IsObject(); }

  v8::Local<v8::Value> V8Value() const {
    if (!isolate_)
      return v8::Local<v8::Value>();
    switch (value_type_) {
      case ValueType::kUndefined:
        return v8::Undefined(isolate_);
      case ValueType::kNull:
        return v8::Null(isolate_);
      case ValueType::kObject:
        return dictionary_object_;
      default:
        NOTREACHED();
        return v8::Local<v8::Value>();
    }
  }

  bool Get(const StringView& key, v8::Local<v8::Value>& value) const {
    return isolate_ && GetInternal(V8String(isolate_, key), value);
  }
  bool Get(const StringView& key, Dictionary&) const;

  HashMap<String, String> GetOwnPropertiesAsStringHashMap(
      ExceptionState&) const;
  Vector<String> GetPropertyNames(ExceptionState&) const;

  bool HasProperty(const StringView& key, ExceptionState&) const;

  v8::Isolate* GetIsolate() const { return isolate_; }
  v8::Local<v8::Context> V8Context() const {
    DCHECK(isolate_);
    return isolate_->GetCurrentContext();
  }

  DictionaryIterator GetIterator(ExecutionContext*) const;

 private:
  bool GetInternal(const v8::Local<v8::Value>& key,
                   v8::Local<v8::Value>& result) const;

  v8::Isolate* isolate_;
  // Undefined, Null, or Object is allowed as type of dictionary.
  enum class ValueType {
    kUndefined,
    kNull,
    kObject
  } value_type_ = ValueType::kUndefined;
  v8::Local<v8::Object> dictionary_object_;  // an Object or empty
};

template <>
struct NativeValueTraits<Dictionary>
    : public NativeValueTraitsBase<Dictionary> {
  static Dictionary NativeValue(v8::Isolate* isolate,
                                v8::Local<v8::Value> value,
                                ExceptionState& exception_state) {
    return Dictionary(isolate, value, exception_state);
  }
};

// DictionaryHelper is a collection of static methods for getting or
// converting a value from Dictionary.
struct DictionaryHelper {
  STATIC_ONLY(DictionaryHelper);
  template <typename T>
  static bool Get(const Dictionary&, const StringView& key, T& value);
  template <typename T>
  static bool Get(const Dictionary&,
                  const StringView& key,
                  T& value,
                  bool& has_value);
  template <typename T>
  static bool Get(const Dictionary&,
                  const StringView& key,
                  T& value,
                  ExceptionState&);
  template <typename T>
  static bool GetWithUndefinedCheck(const Dictionary& dictionary,
                                    const StringView& key,
                                    T& value) {
    v8::Local<v8::Value> v8_value;
    if (!dictionary.Get(key, v8_value) || v8_value.IsEmpty() ||
        v8_value->IsUndefined())
      return false;
    return DictionaryHelper::Get(dictionary, key, value);
  }
  template <template <typename> class PointerType, typename T>
  static bool Get(const Dictionary&,
                  const StringView& key,
                  PointerType<T>& value);
  template <typename T>
  static bool Get(const Dictionary&, const StringView& key, Nullable<T>& value);
};

}  // namespace blink

#endif  // Dictionary_h

// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bindings/core/v8/NativeValueTraitsImpl.h"

#include <utility>
#include "bindings/core/v8/ExceptionState.h"
#include "bindings/core/v8/IDLTypes.h"
#include "bindings/core/v8/ToV8ForCore.h"
#include "bindings/core/v8/V8BindingForTesting.h"
#include "bindings/core/v8/V8Internals.h"
#include "bindings/core/v8/v8_test_sequence_callback.h"
#include "platform/wtf/Vector.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

namespace {

template <typename T>
v8::Local<v8::Value> ToV8(V8TestingScope* scope, T value) {
  return blink::ToV8(value, scope->GetContext()->Global(), scope->GetIsolate());
}

TEST(NativeValueTraitsImplTest, IDLInterface) {
  V8TestingScope scope;
  {
    DummyExceptionStateForTesting exception_state;
    Internals* internals = NativeValueTraits<Internals>::NativeValue(
        scope.GetIsolate(), v8::Number::New(scope.GetIsolate(), 42),
        exception_state);
    EXPECT_TRUE(exception_state.HadException());
    EXPECT_EQ("Failed to convert value to 'Internals'.",
              exception_state.Message());
    EXPECT_EQ(nullptr, internals);
  }
  {
    DummyExceptionStateForTesting exception_state;
    V8TestSequenceCallback* callback_function =
        NativeValueTraits<V8TestSequenceCallback>::NativeValue(
            scope.GetIsolate(), v8::Undefined(scope.GetIsolate()),
            exception_state);
    EXPECT_TRUE(exception_state.HadException());
    EXPECT_EQ("Failed to convert value to 'TestSequenceCallback'.",
              exception_state.Message());
    EXPECT_EQ(nullptr, callback_function);
  }
}

void ThrowException(v8::Local<v8::Name>,
                    const v8::PropertyCallbackInfo<v8::Value>& info) {
  info.GetIsolate()->ThrowException(V8String(info.GetIsolate(), "bogus!"));
}

void ReturnBogusObjectDescriptor(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  auto context = isolate->GetCurrentContext();

  v8::Local<v8::Object> descriptor = v8::Object::New(isolate);
  EXPECT_TRUE(
      V8CallBoolean(descriptor->Set(context, V8String(isolate, "configurable"),
                                    v8::Boolean::New(isolate, true))));
  EXPECT_TRUE(V8CallBoolean(descriptor->SetAccessor(
      context, V8String(isolate, "enumerable"), ThrowException)));
  info.GetReturnValue().Set(descriptor);
}

TEST(NativeValueTraitsImplTest, IDLRecord) {
  V8TestingScope scope;
  {
    v8::Local<v8::Object> v8_object = v8::Object::New(scope.GetIsolate());
    NonThrowableExceptionState exception_state;
    const auto& record =
        NativeValueTraits<IDLRecord<IDLString, IDLOctet>>::NativeValue(
            scope.GetIsolate(), v8_object, exception_state);
    EXPECT_TRUE(record.IsEmpty());
  }
  {
    v8::Local<v8::Object> v8_object = v8::Object::New(scope.GetIsolate());
    EXPECT_TRUE(V8CallBoolean(v8_object->Set(
        scope.GetContext(), ToV8(&scope, "foo"), ToV8(&scope, 34))));
    EXPECT_TRUE(V8CallBoolean(v8_object->Set(
        scope.GetContext(), ToV8(&scope, "bar"), ToV8(&scope, -1024))));
    EXPECT_TRUE(V8CallBoolean(v8_object->Set(
        scope.GetContext(), ToV8(&scope, "foo"), ToV8(&scope, 42))));

    NonThrowableExceptionState exception_state;
    const auto& record =
        NativeValueTraits<IDLRecord<IDLByteString, IDLLong>>::NativeValue(
            scope.GetIsolate(), v8_object, exception_state);
    EXPECT_EQ(2U, record.size());
    EXPECT_EQ(std::make_pair(String("foo"), int32_t(42)), record[0]);
    EXPECT_EQ(std::make_pair(String("bar"), int32_t(-1024)), record[1]);
  }
  {
    v8::Local<v8::Object> v8_object = v8::Object::New(scope.GetIsolate());
    EXPECT_TRUE(V8CallBoolean(v8_object->Set(
        scope.GetContext(), ToV8(&scope, "foo"), ToV8(&scope, 34))));
    EXPECT_TRUE(V8CallBoolean(v8_object->DefineOwnProperty(
        scope.GetContext(), V8String(scope.GetIsolate(), "bar"),
        ToV8(&scope, -1024), v8::PropertyAttribute::DontEnum)));
    EXPECT_TRUE(V8CallBoolean(v8_object->Set(
        scope.GetContext(), ToV8(&scope, "baz"), ToV8(&scope, 42))));

    NonThrowableExceptionState exception_state;
    const auto& record =
        NativeValueTraits<IDLRecord<IDLByteString, IDLLong>>::NativeValue(
            scope.GetIsolate(), v8_object, exception_state);
    EXPECT_EQ(2U, record.size());
    EXPECT_EQ(std::make_pair(String("foo"), int32_t(34)), record[0]);
    EXPECT_EQ(std::make_pair(String("baz"), int32_t(42)), record[1]);
  }
  {
    // Exceptions are being thrown in this test, so we need another scope.
    V8TestingScope scope;
    v8::Local<v8::Object> original_object = v8::Object::New(scope.GetIsolate());
    EXPECT_TRUE(V8CallBoolean(original_object->Set(
        scope.GetContext(), ToV8(&scope, "foo"), ToV8(&scope, 34))));
    EXPECT_TRUE(V8CallBoolean(original_object->Set(
        scope.GetContext(), ToV8(&scope, "bar"), ToV8(&scope, 42))));

    NonThrowableExceptionState exception_state;
    const auto& record =
        NativeValueTraits<IDLRecord<IDLString, IDLLong>>::NativeValue(
            scope.GetIsolate(), original_object, exception_state);
    EXPECT_EQ(2U, record.size());

    v8::Local<v8::Object> handler = v8::Object::New(scope.GetIsolate());
    EXPECT_TRUE(V8CallBoolean(handler->Set(
        scope.GetContext(), ToV8(&scope, "getOwnPropertyDescriptor"),
        v8::Function::New(scope.GetContext(), ReturnBogusObjectDescriptor)
            .ToLocalChecked())));
    v8::Local<v8::Proxy> proxy =
        v8::Proxy::New(scope.GetContext(), original_object, handler)
            .ToLocalChecked();
    ExceptionState exception_state_from_proxy(
        scope.GetIsolate(), ExceptionState::kExecutionContext,
        "NativeValueTraitsImplTest", "IDLRecordTest");
    const auto& record_from_proxy =
        NativeValueTraits<IDLRecord<IDLString, IDLLong>>::NativeValue(
            scope.GetIsolate(), proxy, exception_state_from_proxy);
    EXPECT_EQ(0U, record_from_proxy.size());
    EXPECT_TRUE(exception_state_from_proxy.HadException());
    EXPECT_TRUE(exception_state_from_proxy.Message().IsEmpty());
    v8::Local<v8::Value> v8_exception =
        exception_state_from_proxy.GetException();
    EXPECT_TRUE(v8_exception->IsString());
    EXPECT_TRUE(
        V8String(scope.GetIsolate(), "bogus!")
            ->Equals(
                v8_exception->ToString(scope.GetContext()).ToLocalChecked()));
  }
  {
    v8::Local<v8::Object> v8_object = v8::Object::New(scope.GetIsolate());
    EXPECT_TRUE(V8CallBoolean(v8_object->Set(
        scope.GetContext(), ToV8(&scope, "foo"), ToV8(&scope, 42))));
    EXPECT_TRUE(V8CallBoolean(v8_object->Set(
        scope.GetContext(), ToV8(&scope, "bar"), ToV8(&scope, 0))));
    EXPECT_TRUE(V8CallBoolean(v8_object->Set(
        scope.GetContext(), ToV8(&scope, "xx"), ToV8(&scope, true))));
    EXPECT_TRUE(V8CallBoolean(v8_object->Set(
        scope.GetContext(), ToV8(&scope, "abcd"), ToV8(&scope, false))));

    NonThrowableExceptionState exception_state;
    const auto& record =
        NativeValueTraits<IDLRecord<IDLByteString, IDLBoolean>>::NativeValue(
            scope.GetIsolate(), v8_object, exception_state);
    EXPECT_EQ(4U, record.size());
    EXPECT_EQ(std::make_pair(String("foo"), true), record[0]);
    EXPECT_EQ(std::make_pair(String("bar"), false), record[1]);
    EXPECT_EQ(std::make_pair(String("xx"), true), record[2]);
    EXPECT_EQ(std::make_pair(String("abcd"), false), record[3]);
  }
  {
    v8::Local<v8::Array> v8_string_array =
        v8::Array::New(scope.GetIsolate(), 2);
    EXPECT_TRUE(V8CallBoolean(v8_string_array->Set(
        scope.GetContext(), ToV8(&scope, 0), ToV8(&scope, "Hello, World!"))));
    EXPECT_TRUE(V8CallBoolean(v8_string_array->Set(
        scope.GetContext(), ToV8(&scope, 1), ToV8(&scope, "Hi, Mom!"))));
    EXPECT_TRUE(V8CallBoolean(v8_string_array->Set(
        scope.GetContext(), ToV8(&scope, "foo"), ToV8(&scope, "Ohai"))));

    NonThrowableExceptionState exception_state;
    const auto& record =
        NativeValueTraits<IDLRecord<IDLUSVString, IDLString>>::NativeValue(
            scope.GetIsolate(), v8_string_array, exception_state);
    EXPECT_EQ(3U, record.size());
    EXPECT_EQ(std::make_pair(String("0"), String("Hello, World!")), record[0]);
    EXPECT_EQ(std::make_pair(String("1"), String("Hi, Mom!")), record[1]);
    EXPECT_EQ(std::make_pair(String("foo"), String("Ohai")), record[2]);
  }
  {
    v8::Local<v8::Object> v8_object = v8::Object::New(scope.GetIsolate());
    EXPECT_TRUE(V8CallBoolean(v8_object->Set(
        scope.GetContext(), v8::Symbol::GetToStringTag(scope.GetIsolate()),
        ToV8(&scope, 34))));
    EXPECT_TRUE(V8CallBoolean(v8_object->Set(
        scope.GetContext(), ToV8(&scope, "foo"), ToV8(&scope, 42))));

    // The presence of symbols should throw a TypeError when the conversion to
    // the record's key type is attempted.
    DummyExceptionStateForTesting exception_state;
    const auto& record =
        NativeValueTraits<IDLRecord<IDLString, IDLShort>>::NativeValue(
            scope.GetIsolate(), v8_object, exception_state);
    EXPECT_TRUE(record.IsEmpty());
    EXPECT_TRUE(exception_state.HadException());
    EXPECT_TRUE(exception_state.Message().IsEmpty());
  }
  {
    v8::Local<v8::Object> v8_parent_object =
        v8::Object::New(scope.GetIsolate());
    EXPECT_TRUE(V8CallBoolean(v8_parent_object->Set(
        scope.GetContext(), ToV8(&scope, "foo"), ToV8(&scope, 34))));
    EXPECT_TRUE(V8CallBoolean(v8_parent_object->Set(
        scope.GetContext(), ToV8(&scope, "bar"), ToV8(&scope, 512))));
    v8::Local<v8::Object> v8_object = v8::Object::New(scope.GetIsolate());
    EXPECT_TRUE(V8CallBoolean(
        v8_object->SetPrototype(scope.GetContext(), v8_parent_object)));

    NonThrowableExceptionState exception_state;
    auto record =
        NativeValueTraits<IDLRecord<IDLString, IDLUnsignedLong>>::NativeValue(
            scope.GetIsolate(), v8_object, exception_state);
    EXPECT_TRUE(record.IsEmpty());

    EXPECT_TRUE(V8CallBoolean(v8_object->Set(
        scope.GetContext(), ToV8(&scope, "quux"), ToV8(&scope, 42))));
    EXPECT_TRUE(V8CallBoolean(v8_object->Set(
        scope.GetContext(), ToV8(&scope, "foo"), ToV8(&scope, 1024))));
    record =
        NativeValueTraits<IDLRecord<IDLString, IDLUnsignedLong>>::NativeValue(
            scope.GetIsolate(), v8_object, exception_state);
    EXPECT_EQ(2U, record.size());
    EXPECT_EQ(std::make_pair(String("quux"), uint32_t(42)), record[0]);
    EXPECT_EQ(std::make_pair(String("foo"), uint32_t(1024)), record[1]);
  }
  {
    v8::Local<v8::Array> v8_string_array =
        v8::Array::New(scope.GetIsolate(), 2);
    EXPECT_TRUE(V8CallBoolean(v8_string_array->Set(
        scope.GetContext(), ToV8(&scope, 0), ToV8(&scope, "Hello, World!"))));
    EXPECT_TRUE(V8CallBoolean(v8_string_array->Set(
        scope.GetContext(), ToV8(&scope, 1), ToV8(&scope, "Hi, Mom!"))));
    v8::Local<v8::Object> v8_object = v8::Object::New(scope.GetIsolate());
    EXPECT_TRUE(
        V8CallBoolean(v8_object->Set(scope.GetContext(), ToV8(&scope, "foo"),
                                     ToV8(&scope, v8_string_array))));

    NonThrowableExceptionState exception_state;
    const auto& record =
        NativeValueTraits<IDLRecord<IDLString, IDLSequence<IDLString>>>::
            NativeValue(scope.GetIsolate(), v8_object, exception_state);
    EXPECT_EQ(1U, record.size());
    EXPECT_EQ("foo", record[0].first);
    EXPECT_EQ("Hello, World!", record[0].second[0]);
    EXPECT_EQ("Hi, Mom!", record[0].second[1]);
  }
}

TEST(NativeValueTraitsImplTest, IDLSequence) {
  V8TestingScope scope;
  {
    v8::Local<v8::Array> v8_array = v8::Array::New(scope.GetIsolate());
    NonThrowableExceptionState exception_state;
    const auto& sequence =
        NativeValueTraits<IDLSequence<IDLOctet>>::NativeValue(
            scope.GetIsolate(), v8_array, exception_state);
    EXPECT_TRUE(sequence.IsEmpty());
  }
  {
    v8::Local<v8::Array> v8_array = v8::Array::New(scope.GetIsolate());
    for (int32_t i = 0; i < 5; ++i) {
      v8_array->Set(scope.GetContext(), ToV8(&scope, i), ToV8(&scope, i))
          .ToChecked();
    }
    NonThrowableExceptionState exception_state;
    const auto& sequence = NativeValueTraits<IDLSequence<IDLLong>>::NativeValue(
        scope.GetIsolate(), v8_array, exception_state);
    EXPECT_EQ(Vector<int32_t>({0, 1, 2, 3, 4}), sequence);
  }
  {
    const double double_pi = 3.141592653589793238;
    const float float_pi = double_pi;
    v8::Local<v8::Array> v8_real_array = v8::Array::New(scope.GetIsolate(), 1);
    v8_real_array
        ->Set(scope.GetContext(), ToV8(&scope, 0), ToV8(&scope, double_pi))
        .ToChecked();

    NonThrowableExceptionState exception_state;
    Vector<double> double_vector =
        NativeValueTraits<IDLSequence<IDLDouble>>::NativeValue(
            scope.GetIsolate(), v8_real_array, exception_state);
    EXPECT_EQ(1U, double_vector.size());
    EXPECT_EQ(double_pi, double_vector[0]);

    Vector<float> float_vector =
        NativeValueTraits<IDLSequence<IDLFloat>>::NativeValue(
            scope.GetIsolate(), v8_real_array, exception_state);
    EXPECT_EQ(1U, float_vector.size());
    EXPECT_EQ(float_pi, float_vector[0]);
  }
  {
    v8::Local<v8::Array> v8_array = v8::Array::New(scope.GetIsolate(), 3);
    EXPECT_TRUE(v8_array
                    ->Set(scope.GetContext(), ToV8(&scope, 0),
                          ToV8(&scope, "Vini, vidi, vici."))
                    .ToChecked());
    EXPECT_TRUE(
        v8_array->Set(scope.GetContext(), ToV8(&scope, 1), ToV8(&scope, 65535))
            .ToChecked());
    EXPECT_TRUE(
        v8_array->Set(scope.GetContext(), ToV8(&scope, 2), ToV8(&scope, 0.125))
            .ToChecked());

    NonThrowableExceptionState exception_state;
    Vector<ScriptValue> script_value_vector =
        NativeValueTraits<IDLSequence<ScriptValue>>::NativeValue(
            scope.GetIsolate(), v8_array, exception_state);
    EXPECT_EQ(3U, script_value_vector.size());
    String report_on_zela;
    EXPECT_TRUE(script_value_vector[0].ToString(report_on_zela));
    EXPECT_EQ("Vini, vidi, vici.", report_on_zela);
    EXPECT_EQ(65535U,
              ToUInt32(scope.GetIsolate(), script_value_vector[1].V8Value(),
                       kNormalConversion, exception_state));
  }
  {
    v8::Local<v8::Array> v8_string_array1 =
        v8::Array::New(scope.GetIsolate(), 2);
    EXPECT_TRUE(
        v8_string_array1
            ->Set(scope.GetContext(), ToV8(&scope, 0), ToV8(&scope, "foo"))
            .ToChecked());
    EXPECT_TRUE(
        v8_string_array1
            ->Set(scope.GetContext(), ToV8(&scope, 1), ToV8(&scope, "bar"))
            .ToChecked());
    v8::Local<v8::Array> v8_string_array2 =
        v8::Array::New(scope.GetIsolate(), 3);
    EXPECT_TRUE(
        v8_string_array2
            ->Set(scope.GetContext(), ToV8(&scope, 0), ToV8(&scope, "x"))
            .ToChecked());
    EXPECT_TRUE(
        v8_string_array2
            ->Set(scope.GetContext(), ToV8(&scope, 1), ToV8(&scope, "y"))
            .ToChecked());
    EXPECT_TRUE(
        v8_string_array2
            ->Set(scope.GetContext(), ToV8(&scope, 2), ToV8(&scope, "z"))
            .ToChecked());
    v8::Local<v8::Array> v8_string_array_array =
        v8::Array::New(scope.GetIsolate(), 2);
    EXPECT_TRUE(v8_string_array_array
                    ->Set(scope.GetContext(), ToV8(&scope, 0), v8_string_array1)
                    .ToChecked());
    EXPECT_TRUE(v8_string_array_array
                    ->Set(scope.GetContext(), ToV8(&scope, 1), v8_string_array2)
                    .ToChecked());

    NonThrowableExceptionState exception_state;
    Vector<Vector<String>> string_vector_vector =
        NativeValueTraits<IDLSequence<IDLSequence<IDLString>>>::NativeValue(
            scope.GetIsolate(), v8_string_array_array, exception_state);
    EXPECT_EQ(2U, string_vector_vector.size());
    EXPECT_EQ(2U, string_vector_vector[0].size());
    EXPECT_EQ("foo", string_vector_vector[0][0]);
    EXPECT_EQ("bar", string_vector_vector[0][1]);
    EXPECT_EQ(3U, string_vector_vector[1].size());
    EXPECT_EQ("x", string_vector_vector[1][0]);
    EXPECT_EQ("y", string_vector_vector[1][1]);
    EXPECT_EQ("z", string_vector_vector[1][2]);
  }
  {
    v8::Local<v8::String> script_code =
        ToV8(&scope,
             "let arr = [1, 2, 3];"
             "let iterations = ["
             "  {done: false, value: 8},"
             "  {done: false, value: 5},"
             "  {done: true}"
             "];"
             "arr[Symbol.iterator] = function() {"
             "  let i = 0;"
             "  return {next: () => iterations[i++]};"
             "}; arr")
            .As<v8::String>();
    v8::MicrotasksScope microtasks(scope.GetIsolate(),
                                   v8::MicrotasksScope::kDoNotRunMicrotasks);
    v8::Local<v8::Value> v8_array =
        v8::Script::Compile(scope.GetContext(), script_code)
            .ToLocalChecked()
            ->Run(scope.GetContext())
            .ToLocalChecked();
    EXPECT_TRUE(v8_array->IsArray());

    NonThrowableExceptionState exception_state;
    const auto& sequence = NativeValueTraits<IDLSequence<IDLByte>>::NativeValue(
        scope.GetIsolate(), v8_array, exception_state);
    EXPECT_EQ(Vector<int8_t>({1, 2, 3}), sequence);
  }
  {
    v8::Local<v8::String> script_code =
        ToV8(&scope,
             "let obj = {"
             "  iterations: ["
             "    {done: false, value: 55},"
             "    {done: false, value: 0},"
             "    {done: true, value: 99}"
             "  ],"
             "  [Symbol.iterator]() {"
             "    let i = 0;"
             "    return {next: () => this.iterations[i++]};"
             "  }"
             "}; obj")
            .As<v8::String>();
    v8::MicrotasksScope microtasks(scope.GetIsolate(),
                                   v8::MicrotasksScope::kDoNotRunMicrotasks);
    v8::Local<v8::Value> v8_object =
        v8::Script::Compile(scope.GetContext(), script_code)
            .ToLocalChecked()
            ->Run(scope.GetContext())
            .ToLocalChecked();
    EXPECT_TRUE(v8_object->IsObject());

    NonThrowableExceptionState exception_state;
    const auto& byte_sequence =
        NativeValueTraits<IDLSequence<IDLByte>>::NativeValue(
            scope.GetIsolate(), v8_object, exception_state);
    EXPECT_EQ(Vector<int8_t>({55, 0}), byte_sequence);
    const auto& boolean_sequence =
        NativeValueTraits<IDLSequence<IDLBoolean>>::NativeValue(
            scope.GetIsolate(), v8_object, exception_state);
    EXPECT_EQ(Vector<bool>({true, false}), boolean_sequence);
  }
}

}  // namespace

}  // namespace blink

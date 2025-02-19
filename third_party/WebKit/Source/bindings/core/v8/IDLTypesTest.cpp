// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bindings/core/v8/IDLTypes.h"

#include <type_traits>
#include "bindings/core/v8/V8InternalDictionary.h"
#include "bindings/core/v8/dictionary_sequence_or_dictionary.h"
#include "core/dom/Element.h"
#include "platform/heap/Handle.h"

// No gtest tests; only static_assert checks.

namespace blink {

namespace {

static_assert(std::is_base_of<IDLBase, IDLBoolean>::value,
              "IDLBoolean inherits from IDLBase");
static_assert(std::is_same<IDLBoolean::ImplType, bool>::value,
              "IDLBoolean's ImplType is bool");

static_assert(std::is_base_of<IDLBase, IDLByte>::value,
              "IDLByte inherits from IDLBase");
static_assert(std::is_same<IDLByte::ImplType, int8_t>::value,
              "IDLByte's ImplType is int8_t");

static_assert(std::is_base_of<IDLBase, IDLOctet>::value,
              "IDLOctet inherits from IDLBase");
static_assert(std::is_same<IDLOctet::ImplType, uint8_t>::value,
              "IDLOctet's ImplType is int16_t");

static_assert(std::is_base_of<IDLBase, IDLShort>::value,
              "IDLShort inherits from IDLBase");
static_assert(std::is_same<IDLShort::ImplType, int16_t>::value,
              "IDLShort's ImplType is uint16_t");

static_assert(std::is_base_of<IDLBase, IDLUnsignedShort>::value,
              "IDLUnsignedShort inherits from IDLBase");
static_assert(std::is_same<IDLUnsignedShort::ImplType, uint16_t>::value,
              "IDLUnsignedShort's ImplType is uint16_t");

static_assert(std::is_base_of<IDLBase, IDLLong>::value,
              "IDLLong inherits from IDLBase");
static_assert(std::is_same<IDLLong::ImplType, int32_t>::value,
              "IDLLong's ImplType is int32_t");

static_assert(std::is_base_of<IDLBase, IDLUnsignedLong>::value,
              "IDLUnsignedLong inherits from IDLBase");
static_assert(std::is_same<IDLUnsignedLong::ImplType, uint32_t>::value,
              "IDLUnsignedLong's ImplType is uint32_t");

static_assert(std::is_base_of<IDLBase, IDLLongLong>::value,
              "IDLLongLong inherits from IDLBase");
static_assert(std::is_same<IDLLongLong::ImplType, int64_t>::value,
              "IDLLongLong's ImplType is int64_t");

static_assert(std::is_base_of<IDLBase, IDLUnsignedLongLong>::value,
              "IDLUnsignedLongLong inherits from IDLBase");
static_assert(std::is_same<IDLUnsignedLongLong::ImplType, uint64_t>::value,
              "IDLUnsignedLongLong's ImplType is uint64_t");

static_assert(std::is_base_of<IDLBase, IDLByteString>::value,
              "IDLByteString inherits from IDLBase");
static_assert(std::is_same<IDLByteString::ImplType, String>::value,
              "IDLByteString's ImplType is String");

static_assert(std::is_base_of<IDLBase, IDLString>::value,
              "IDLString inherits from IDLBase");
static_assert(std::is_same<IDLString::ImplType, String>::value,
              "IDLString's ImplType is String");

static_assert(std::is_base_of<IDLBase, IDLUSVString>::value,
              "IDLUSVString inherits from IDLBase");
static_assert(std::is_same<IDLUSVString::ImplType, String>::value,
              "IDLUSVString's ImplType is String");

static_assert(std::is_base_of<IDLBase, IDLDouble>::value,
              "IDLDouble inherits from IDLBase");
static_assert(std::is_same<IDLDouble::ImplType, double>::value,
              "IDLDouble's ImplType is double");

static_assert(std::is_base_of<IDLBase, IDLUnrestrictedDouble>::value,
              "IDLUnrestrictedDouble inherits from IDLBase");
static_assert(std::is_same<IDLUnrestrictedDouble::ImplType, double>::value,
              "IDLUnrestrictedDouble's ImplType is double");

static_assert(std::is_base_of<IDLBase, IDLFloat>::value,
              "IDLFloat inherits from IDLBase");
static_assert(std::is_same<IDLFloat::ImplType, float>::value,
              "IDLFloat's ImplType is float");

static_assert(std::is_base_of<IDLBase, IDLUnrestrictedFloat>::value,
              "IDLUnrestrictedFloat inherits from IDLBase");
static_assert(std::is_same<IDLUnrestrictedFloat::ImplType, float>::value,
              "IDLUnrestrictedFloat's ImplType is float");

static_assert(std::is_base_of<IDLBase, IDLPromise>::value,
              "IDLPromise inherits from IDLBase");
static_assert(std::is_same<IDLPromise::ImplType, ScriptPromise>::value,
              "IDLPromise's ImplType is ScriptPromise");

static_assert(std::is_base_of<IDLBase, IDLSequence<IDLByte>>::value,
              "IDLSequence inherits from IDLBase");
static_assert(
    std::is_same<IDLSequence<IDLByte>::ImplType, Vector<int8_t>>::value,
    "IDLSequence<IDLByte> produces a Vector");
static_assert(std::is_same<IDLSequence<Element>::ImplType,
                           HeapVector<Member<Element>>>::value,
              "IDLSequence<GC-type>> produces a HeapVector<Member<>>");
static_assert(std::is_same<IDLSequence<InternalDictionary>::ImplType,
                           HeapVector<InternalDictionary>>::value,
              "IDLSequence<dictionary type> produces a HeapVector");
static_assert(
    std::is_same<IDLSequence<DictionarySequenceOrDictionary>::ImplType,
                 HeapVector<DictionarySequenceOrDictionary>>::value,
    "IDLSequence<union type> produces a HeapVector");

static_assert(std::is_base_of<IDLBase, IDLRecord<IDLString, IDLShort>>::value,
              "IDLRecord inherits from IDLBase");
static_assert(std::is_base_of<IDLBase, IDLRecord<IDLString, Element>>::value,
              "IDLRecord inherits from IDLBase");
static_assert(std::is_same<IDLRecord<IDLByteString, IDLLong>::ImplType,
                           Vector<std::pair<String, int32_t>>>::value,
              "IDLRecord<IDLByteString, IDLLong> produces a Vector");
static_assert(
    std::is_same<IDLRecord<IDLByteString, Element>::ImplType,
                 HeapVector<std::pair<String, Member<Element>>>>::value,
    "IDLRecord<IDLByteString, GC-type>> produces a HeapVector with Member<>");
static_assert(
    std::is_same<IDLRecord<IDLUSVString, InternalDictionary>::ImplType,
                 HeapVector<std::pair<String, InternalDictionary>>>::value,
    "IDLRecord<IDLUSVString, dictionary type> produces a HeapVector with no "
    "Member<>");
static_assert(
    std::is_same<
        IDLRecord<IDLString, DictionarySequenceOrDictionary>::ImplType,
        HeapVector<std::pair<String, DictionarySequenceOrDictionary>>>::value,
    "IDLRecord<IDLString, union type> produces a HeapVector with no Member<>");

}  // namespace

}  // namespace blink

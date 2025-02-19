// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "RecordTest.h"

namespace blink {

RecordTest::RecordTest() {}

RecordTest::~RecordTest() {}

void RecordTest::setStringLongRecord(
    const Vector<std::pair<String, int32_t>>& arg) {
  string_long_record_ = arg;
}

Vector<std::pair<String, int32_t>> RecordTest::getStringLongRecord() {
  return string_long_record_;
}

void RecordTest::setNullableStringLongRecord(
    const Nullable<Vector<std::pair<String, int32_t>>>& arg) {
  nullable_string_long_record_ = arg;
}

Nullable<Vector<std::pair<String, int32_t>>>
RecordTest::getNullableStringLongRecord() {
  return nullable_string_long_record_;
}

Vector<std::pair<String, String>> RecordTest::GetByteStringByteStringRecord() {
  return byte_string_byte_string_record_;
}

void RecordTest::setByteStringByteStringRecord(
    const Vector<std::pair<String, String>>& arg) {
  byte_string_byte_string_record_ = arg;
}

void RecordTest::setStringElementRecord(
    const HeapVector<std::pair<String, Member<Element>>>& arg) {
  string_element_record_ = arg;
}

HeapVector<std::pair<String, Member<Element>>>
RecordTest::getStringElementRecord() {
  return string_element_record_;
}

void RecordTest::setUSVStringUSVStringBooleanRecordRecord(
    const RecordTest::NestedRecordType& arg) {
  usv_string_usv_string_boolean_record_record_ = arg;
}

RecordTest::NestedRecordType
RecordTest::getUSVStringUSVStringBooleanRecordRecord() {
  return usv_string_usv_string_boolean_record_record_;
}

Vector<std::pair<String, Vector<String>>>
RecordTest::returnStringByteStringSequenceRecord() {
  Vector<std::pair<String, Vector<String>>> record;
  Vector<String> inner_vector1;
  inner_vector1.push_back("hello, world");
  inner_vector1.push_back("hi, mom");
  record.push_back(std::make_pair(String("foo"), inner_vector1));
  Vector<String> inner_vector2;
  inner_vector2.push_back("goodbye, mom");
  record.push_back(std::make_pair(String("bar"), inner_vector2));
  return record;
}

bool RecordTest::unionReceivedARecord(
    const BooleanOrByteStringByteStringRecord& arg) {
  return arg.isByteStringByteStringRecord();
}

DEFINE_TRACE(RecordTest) {
  visitor->Trace(string_element_record_);
}

}  // namespace blink

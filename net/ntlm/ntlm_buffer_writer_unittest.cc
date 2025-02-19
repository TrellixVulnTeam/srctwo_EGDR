// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/ntlm/ntlm_buffer_writer.h"

#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace net {
namespace ntlm {

namespace {

// Helper method to get a raw pointer to the buffer.
const uint8_t* GetBufferPtr(const NtlmBufferWriter& writer) {
  return writer.GetBuffer().data();
}

// Helper method to get a byte at a specific index in the buffer.
uint8_t GetByteFromBuffer(const NtlmBufferWriter& writer, size_t index) {
  EXPECT_TRUE(index < writer.GetLength());
  return writer.GetBuffer()[index];
}

}  // namespace

TEST(NtlmBufferWriterTest, Initialization) {
  NtlmBufferWriter writer(1);

  ASSERT_EQ(1u, writer.GetLength());
  ASSERT_EQ(1u, writer.GetBuffer().length());
  ASSERT_EQ(0u, writer.GetCursor());
  ASSERT_FALSE(writer.IsEndOfBuffer());
  ASSERT_TRUE(writer.CanWrite(1));
  ASSERT_FALSE(writer.CanWrite(2));
}

TEST(NtlmBufferWriterTest, Write16) {
  uint8_t expected[2] = {0x22, 0x11};
  const uint16_t value = 0x1122;

  NtlmBufferWriter writer(sizeof(uint16_t));

  ASSERT_TRUE(writer.WriteUInt16(value));
  ASSERT_TRUE(writer.IsEndOfBuffer());
  ASSERT_EQ(arraysize(expected), writer.GetLength());
  ASSERT_FALSE(writer.WriteUInt16(value));

  ASSERT_EQ(0,
            memcmp(expected, writer.GetBuffer().data(), arraysize(expected)));
}

TEST(NtlmBufferWriterTest, Write16PastEob) {
  NtlmBufferWriter writer(sizeof(uint16_t) - 1);

  ASSERT_FALSE(writer.WriteUInt16(0));
  ASSERT_EQ(0u, writer.GetCursor());
}

TEST(NtlmBufferWriterTest, Write32) {
  uint8_t expected[4] = {0x44, 0x33, 0x22, 0x11};
  const uint32_t value = 0x11223344;

  NtlmBufferWriter writer(sizeof(uint32_t));

  ASSERT_TRUE(writer.WriteUInt32(value));
  ASSERT_TRUE(writer.IsEndOfBuffer());
  ASSERT_FALSE(writer.WriteUInt32(value));

  ASSERT_EQ(0, memcmp(expected, GetBufferPtr(writer), arraysize(expected)));
}

TEST(NtlmBufferWriterTest, Write32PastEob) {
  NtlmBufferWriter writer(sizeof(uint32_t) - 1);

  ASSERT_FALSE(writer.WriteUInt32(0));
  ASSERT_EQ(0u, writer.GetCursor());
}

TEST(NtlmBufferWriterTest, Write64) {
  uint8_t expected[8] = {0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11};
  const uint64_t value = 0x1122334455667788;

  NtlmBufferWriter writer(sizeof(uint64_t));

  ASSERT_TRUE(writer.WriteUInt64(value));
  ASSERT_TRUE(writer.IsEndOfBuffer());
  ASSERT_FALSE(writer.WriteUInt64(value));

  ASSERT_EQ(0, memcmp(expected, GetBufferPtr(writer), arraysize(expected)));
}

TEST(NtlmBufferWriterTest, Write64PastEob) {
  NtlmBufferWriter writer(sizeof(uint64_t) - 1);

  ASSERT_FALSE(writer.WriteUInt64(0));
  ASSERT_EQ(0u, writer.GetCursor());
}

TEST(NtlmBufferWriterTest, WriteBytes) {
  uint8_t expected[8] = {0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11};

  NtlmBufferWriter writer(arraysize(expected));

  ASSERT_TRUE(writer.WriteBytes(expected, arraysize(expected)));
  ASSERT_EQ(0, memcmp(GetBufferPtr(writer), expected, arraysize(expected)));
  ASSERT_TRUE(writer.IsEndOfBuffer());
  ASSERT_FALSE(writer.WriteBytes(expected, 1));

  ASSERT_EQ(0, memcmp(expected, GetBufferPtr(writer), arraysize(expected)));
}

TEST(NtlmBufferWriterTest, WriteBytesPastEob) {
  uint8_t buffer[8];

  NtlmBufferWriter writer(arraysize(buffer) - 1);

  ASSERT_FALSE(writer.WriteBytes(buffer, arraysize(buffer)));
}

TEST(NtlmBufferWriterTest, WriteSecurityBuffer) {
  uint8_t expected[8] = {0x22, 0x11, 0x22, 0x11, 0x88, 0x77, 0x66, 0x55};
  uint16_t length = 0x1122;
  uint32_t offset = 0x55667788;

  NtlmBufferWriter writer(kSecurityBufferLen);

  ASSERT_TRUE(writer.WriteSecurityBuffer(SecurityBuffer(offset, length)));
  ASSERT_TRUE(writer.IsEndOfBuffer());
  ASSERT_FALSE(writer.WriteSecurityBuffer(SecurityBuffer(offset, length)));

  ASSERT_EQ(0, memcmp(expected, GetBufferPtr(writer), arraysize(expected)));
}

TEST(NtlmBufferWriterTest, WriteSecurityBufferPastEob) {
  SecurityBuffer sec_buf;
  NtlmBufferWriter writer(kSecurityBufferLen - 1);

  ASSERT_FALSE(writer.WriteSecurityBuffer(sec_buf));
}

TEST(NtlmBufferWriterTest, WriteNarrowString) {
  uint8_t expected[8] = {'1', '2', '3', '4', '5', '6', '7', '8'};
  std::string value("12345678");

  NtlmBufferWriter writer(value.size());

  ASSERT_TRUE(writer.WriteUtf8String(value));
  ASSERT_TRUE(writer.IsEndOfBuffer());
  ASSERT_FALSE(writer.WriteUtf8String(value));

  ASSERT_EQ(0, memcmp(expected, GetBufferPtr(writer), arraysize(expected)));
}

TEST(NtlmBufferWriterTest, WriteAsciiStringPastEob) {
  std::string str("12345678");
  NtlmBufferWriter writer(str.length() - 1);

  ASSERT_FALSE(writer.WriteUtf8String(str));
}

TEST(NtlmBufferWriterTest, WriteUtf16String) {
  uint8_t expected[16] = {'1', 0, '2', 0, '3', 0, '4', 0,
                          '5', 0, '6', 0, '7', 0, '8', 0};
  base::string16 value = base::ASCIIToUTF16("12345678");

  NtlmBufferWriter writer(value.size() * 2);

  ASSERT_TRUE(writer.WriteUtf16String(value));
  ASSERT_TRUE(writer.IsEndOfBuffer());
  ASSERT_FALSE(writer.WriteUtf16String(value));

  ASSERT_EQ(0, memcmp(expected, GetBufferPtr(writer), arraysize(expected)));
}

TEST(NtlmBufferWriterTest, WriteUtf16StringPastEob) {
  base::string16 str = base::ASCIIToUTF16("12345678");
  NtlmBufferWriter writer((str.length() * 2) - 1);

  ASSERT_FALSE(writer.WriteUtf16String(str));
}

TEST(NtlmBufferWriterTest, WriteUtf8AsUtf16String) {
  uint8_t expected[16] = {'1', 0, '2', 0, '3', 0, '4', 0,
                          '5', 0, '6', 0, '7', 0, '8', 0};
  std::string input = "12345678";

  NtlmBufferWriter writer(input.size() * 2);

  ASSERT_TRUE(writer.WriteUtf8AsUtf16String(input));
  ASSERT_TRUE(writer.IsEndOfBuffer());
  ASSERT_FALSE(writer.WriteUtf8AsUtf16String(input));

  ASSERT_EQ(0, memcmp(expected, GetBufferPtr(writer), arraysize(expected)));
}

TEST(NtlmBufferWriterTest, WriteSignature) {
  uint8_t expected[8] = {'N', 'T', 'L', 'M', 'S', 'S', 'P', 0};
  NtlmBufferWriter writer(kSignatureLen);

  ASSERT_TRUE(writer.WriteSignature());
  ASSERT_TRUE(writer.IsEndOfBuffer());

  ASSERT_EQ(0, memcmp(expected, GetBufferPtr(writer), arraysize(expected)));
}

TEST(NtlmBufferWriterTest, WriteSignaturePastEob) {
  NtlmBufferWriter writer(1);

  ASSERT_FALSE(writer.WriteSignature());
}

TEST(NtlmBufferWriterTest, WriteMessageType) {
  NtlmBufferWriter writer(4);

  ASSERT_TRUE(writer.WriteMessageType(MessageType::kNegotiate));
  ASSERT_TRUE(writer.IsEndOfBuffer());
  ASSERT_EQ(static_cast<uint32_t>(MessageType::kNegotiate),
            GetByteFromBuffer(writer, 0));
  ASSERT_EQ(0, GetByteFromBuffer(writer, 1));
  ASSERT_EQ(0, GetByteFromBuffer(writer, 2));
  ASSERT_EQ(0, GetByteFromBuffer(writer, 3));
}

TEST(NtlmBufferWriterTest, WriteMessageTypePastEob) {
  NtlmBufferWriter writer(sizeof(uint32_t) - 1);

  ASSERT_FALSE(writer.WriteMessageType(MessageType::kNegotiate));
}

}  // namespace ntlm
}  // namespace net

// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_BASE_NTLM_BUFFER_WRITER_H_
#define NET_BASE_NTLM_BUFFER_WRITER_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <string>

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "net/base/net_export.h"
#include "net/ntlm/ntlm_constants.h"

namespace net {
namespace ntlm {

// Supports various bounds checked low level buffer operations required by an
// NTLM implementation.
//
// The class supports sequential write to an internally managed buffer. All
// writes perform bounds checking to ensure enough space is remaining in the
// buffer.
//
// The internal buffer is allocated in the constructor with size |buffer_len|
// and owned by the class.
//
// Write* methods write the buffer at the current cursor position and perform
// any necessary type conversion and provide the data in out params. After a
// successful write the cursor position is advanced past the written field.
//
// Failed writes leave the internal cursor at the same position as before the
// call.
//
// Based on [MS-NLMP]: NT LAN Manager (NTLM) Authentication Protocol
// Specification version 28.0 [1]. Additional NTLM reference [2].
//
// [1] https://msdn.microsoft.com/en-us/library/cc236621.aspx
// [2] http://davenport.sourceforge.net/ntlm.html
class NET_EXPORT_PRIVATE NtlmBufferWriter {
 public:
  explicit NtlmBufferWriter(size_t buffer_len);
  ~NtlmBufferWriter();

  size_t GetLength() const { return buffer_.size(); }
  size_t GetCursor() const { return cursor_; }
  bool IsEndOfBuffer() const { return cursor_ >= GetLength(); }
  const Buffer& GetBuffer() const { return buffer_; }
  Buffer Pass() const { return std::move(buffer_); }

  // Returns true if there are |len| more bytes between the current cursor
  // position and the end of the buffer.
  bool CanWrite(size_t len) const;

  // Writes a 16 bit unsigned value (little endian). If there are not 16
  // more bits available in the buffer, it returns false.
  bool WriteUInt16(uint16_t value) WARN_UNUSED_RESULT;

  // Writes a 32 bit unsigned value (little endian). If there are not 32
  // more bits available in the buffer, it returns false.
  bool WriteUInt32(uint32_t value) WARN_UNUSED_RESULT;

  // Writes a 64 bit unsigned value (little endian). If there are not 64
  // more bits available in the buffer, it returns false.
  bool WriteUInt64(uint64_t value) WARN_UNUSED_RESULT;

  // Writes flags as a 32 bit unsigned value (little endian).
  bool WriteFlags(NegotiateFlags flags) WARN_UNUSED_RESULT;

  // Writes |len| bytes from |buffer|. If there are not |len| more bytes in
  // the buffer, it returns false.
  bool WriteBytes(const uint8_t* buffer, size_t len) WARN_UNUSED_RESULT;

  // Writes the bytes from the |base::StringPiece|. If there are not enough
  // bytes in the buffer, it returns false.
  bool WriteBytes(base::StringPiece bytes) WARN_UNUSED_RESULT;

  // Writes |count| bytes of zeros to the buffer. If there are not |count|
  // more bytes in available in the buffer, it returns false.
  bool WriteZeros(size_t count) WARN_UNUSED_RESULT;

  // A security buffer is an 8 byte structure that defines the offset and
  // length of a payload (string, struct, or byte array) that appears after
  // the fixed part of the message.
  //
  // The structure in the NTLM message is (little endian fields):
  //     uint16 - |length| Length of payload
  //     uint16 - Allocation (ignored and always set to |length|)
  //     uint32 - |offset| Offset from start of message
  bool WriteSecurityBuffer(SecurityBuffer sec_buf) WARN_UNUSED_RESULT;

  // Writes a string of 8 bit characters to the buffer.
  //
  // When Unicode was not negotiated only the hostname string will go through
  // this code path. The 8 bit bytes of the hostname are written to the buffer.
  // The encoding is not relevant.
  bool WriteUtf8String(const std::string& str) WARN_UNUSED_RESULT;

  // Converts the 16 bit characters to UTF8 and writes the resulting 8 bit
  // characters.
  //
  // If Unicode was not negotiated, the username and domain get converted to
  // UTF8 in the message. Since they are just treated as additional bytes
  // input to hash the encoding doesn't matter. In practice, only a very old or
  // non-Windows server might trigger this code path since we always attempt
  // to negotiate Unicode and servers are supposed to honor that request.
  bool WriteUtf16AsUtf8String(const base::string16& str) WARN_UNUSED_RESULT;

  // Treats |str| as UTF8, converts to UTF-16 and writes it with little-endian
  // byte order to the buffer.
  //
  // Two specific strings go through this code path.
  //
  // One case is the hostname. When the the Unicode flag has been set during
  // negotiation, the hostname needs to appear in the message with 16-bit
  // characters.
  //
  // The other case is the Service Principal Name (SPN). With Extended
  // Protection for Authentication (EPA) enabled, it appears in the target info
  // inside an AV Pair, where strings always have 16-bit characters.
  bool WriteUtf8AsUtf16String(const std::string& str) WARN_UNUSED_RESULT;

  // Writes UTF-16 LE characters to the buffer. For these strings, such as
  // username and the domain the actual encoding isn't important; they are just
  // treated as additional bytes of input to the hash.
  bool WriteUtf16String(const base::string16& str) WARN_UNUSED_RESULT;

  // Writes the 8 byte NTLM signature "NTLMSSP\0" into the buffer.
  bool WriteSignature() WARN_UNUSED_RESULT;

  // There are 3 message types Negotiate (sent by client), Challenge (sent by
  // server), and Authenticate (sent by client).
  //
  // This writes |message_type| as a uint32_t into the buffer.
  bool WriteMessageType(MessageType message_type) WARN_UNUSED_RESULT;

  // Performs |WriteSignature| then |WriteMessageType|.
  bool WriteMessageHeader(MessageType message_type) WARN_UNUSED_RESULT;

 private:
  // Writes |sizeof(T)| bytes little-endian of an integer type to the buffer.
  template <typename T>
  bool WriteUInt(T value);

  // Sets the cursor position. The caller should use |GetLength| or
  // |CanWrite| to verify the bounds before calling this method.
  void SetCursor(size_t cursor);

  // Advances the cursor by |count|. The caller should use |GetLength| or
  // |CanWrite| to verify the bounds before calling this method.
  void AdvanceCursor(size_t count) { SetCursor(GetCursor() + count); }

  // Returns a pointer to the start of the buffer.
  const uint8_t* GetBufferPtr() const { return &buffer_[0]; }
  uint8_t* GetBufferPtr() { return &buffer_[0]; }

  // Returns pointer into the buffer at the current cursor location.
  const uint8_t* GetBufferPtrAtCursor() const {
    return GetBufferPtr() + GetCursor();
  }
  uint8_t* GetBufferPtrAtCursor() { return GetBufferPtr() + GetCursor(); }

  Buffer buffer_;
  size_t cursor_;

  DISALLOW_COPY_AND_ASSIGN(NtlmBufferWriter);
};

}  // namespace ntlm
}  // namespace net

#endif  // NET_BASE_NTLM_BUFFER_WRITER_H_

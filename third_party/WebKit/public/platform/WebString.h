/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WebString_h
#define WebString_h

#include "WebCommon.h"
#include "WebPrivatePtr.h"
#include "base/strings/latin1_string_conversions.h"
#include "base/strings/nullable_string16.h"
#include "base/strings/string16.h"
#include <string>

#if INSIDE_BLINK
#include "platform/wtf/Forward.h"
#endif

namespace WTF {
class StringImpl;
}

namespace blink {

// Use either one of static methods to convert ASCII, Latin1, UTF-8 or
// UTF-16 string into WebString:
//
// * WebString::FromASCII(const std::string& ascii)
// * WebString::FromLatin1(const std::string& latin1)
// * WebString::FromUTF8(const std::string& utf8)
// * WebString::FromUTF16(const base::string16& utf16)
// * WebString::FromUTF16(const base::NullableString16& utf16)
//
// Similarly, use either of following methods to convert WebString to
// ASCII, Latin1, UTF-8 or UTF-16:
//
// * webstring.Ascii()
// * webstring.Latin1()
// * webstring.Utf8()
// * webstring.Utf16()
// * WebString::toNullableString16(webstring)
//
// Note that if you need to convert the UTF8 string converted from WebString
// back to WebString with FromUTF8() you may want to specify Strict
// UTF8ConversionMode when you call Utf8(), as FromUTF8 rejects strings
// with invalid UTF8 characters.
//
// Some types like GURL and base::FilePath can directly take either utf-8 or
// utf-16 strings. Use following methods to convert WebString to/from GURL or
// FilePath rather than going through intermediate string types:
//
// * GURL WebStringToGURL(const WebString&)
// * base::FilePath WebStringToFilePath(const WebString&)
// * WebString FilePathToWebString(const base::FilePath&);
//
// It is inexpensive to copy a WebString object.
// WARNING: It is not safe to pass a WebString across threads!!!
//
class WebString {
 public:
  enum class UTF8ConversionMode {
    // Ignores errors for invalid characters.
    kLenient,
    // Errors out on invalid characters, returns null string.
    kStrict,
    // Replace invalid characters with 0xFFFD.
    // (This is the same conversion mode as base::UTF16ToUTF8)
    kStrictReplacingErrorsWithFFFD,
  };

  ~WebString() { Reset(); }

  WebString() {}

  WebString(const WebUChar* data, size_t len) { Assign(data, len); }

  WebString(const WebString& s) { Assign(s); }

  WebString& operator=(const WebString& s) {
    Assign(s);
    return *this;
  }

  BLINK_PLATFORM_EXPORT void Reset();
  BLINK_PLATFORM_EXPORT void Assign(const WebString&);
  BLINK_PLATFORM_EXPORT void Assign(const WebUChar* data, size_t len);

  BLINK_PLATFORM_EXPORT bool Equals(const WebString&) const;
  BLINK_PLATFORM_EXPORT bool Equals(const char* characters, size_t len) const;
  bool Equals(const char* characters) const {
    return Equals(characters, characters ? strlen(characters) : 0);
  }

  BLINK_PLATFORM_EXPORT size_t length() const;

  bool IsEmpty() const { return !length(); }
  bool IsNull() const { return private_.IsNull(); }

  BLINK_PLATFORM_EXPORT std::string Utf8(
      UTF8ConversionMode = UTF8ConversionMode::kLenient) const;

  BLINK_PLATFORM_EXPORT static WebString FromUTF8(const char* data,
                                                  size_t length);
  static WebString FromUTF8(const std::string& s) {
    return FromUTF8(s.data(), s.length());
  }

  base::string16 Utf16() const {
    return base::Latin1OrUTF16ToUTF16(length(), Data8(), Data16());
  }

  BLINK_PLATFORM_EXPORT static WebString FromUTF16(const base::string16&);

  BLINK_PLATFORM_EXPORT static WebString FromUTF16(
      const base::NullableString16&);

  static base::NullableString16 ToNullableString16(const WebString& s) {
    return base::NullableString16(s.Utf16(), s.IsNull());
  }

  BLINK_PLATFORM_EXPORT std::string Latin1() const;

  BLINK_PLATFORM_EXPORT static WebString FromLatin1(const WebLChar* data,
                                                    size_t length);

  static WebString FromLatin1(const std::string& s) {
    return FromLatin1(reinterpret_cast<const WebLChar*>(s.data()), s.length());
  }

  // This asserts if the string contains non-ascii characters.
  // Use this rather than calling base::UTF16ToASCII() which always incurs
  // (likely unnecessary) string16 conversion.
  BLINK_PLATFORM_EXPORT std::string Ascii() const;

  // Use this rather than calling base::IsStringASCII().
  BLINK_PLATFORM_EXPORT bool ContainsOnlyASCII() const;

  // Does same as FromLatin1 but asserts if the given string has non-ascii char.
  BLINK_PLATFORM_EXPORT static WebString FromASCII(const std::string&);

  template <int N>
  WebString(const char (&data)[N]) {
    Assign(FromUTF8(data, N - 1));
  }

  template <int N>
  WebString& operator=(const char (&data)[N]) {
    Assign(FromUTF8(data, N - 1));
    return *this;
  }

#if INSIDE_BLINK
  BLINK_PLATFORM_EXPORT WebString(const WTF::String&);
  BLINK_PLATFORM_EXPORT WebString& operator=(const WTF::String&);
  BLINK_PLATFORM_EXPORT operator WTF::String() const;

  BLINK_PLATFORM_EXPORT operator WTF::StringView() const;

  BLINK_PLATFORM_EXPORT WebString(const WTF::AtomicString&);
  BLINK_PLATFORM_EXPORT WebString& operator=(const WTF::AtomicString&);
  BLINK_PLATFORM_EXPORT operator WTF::AtomicString() const;
#endif

 private:
  BLINK_PLATFORM_EXPORT bool Is8Bit() const;
  BLINK_PLATFORM_EXPORT const WebLChar* Data8() const;
  BLINK_PLATFORM_EXPORT const WebUChar* Data16() const;

  BLINK_PLATFORM_EXPORT void Assign(WTF::StringImpl*);

  WebPrivatePtr<WTF::StringImpl> private_;
};

inline bool operator==(const WebString& a, const char* b) {
  return a.Equals(b);
}

inline bool operator!=(const WebString& a, const char* b) {
  return !(a == b);
}

inline bool operator==(const char* a, const WebString& b) {
  return b == a;
}

inline bool operator!=(const char* a, const WebString& b) {
  return !(b == a);
}

inline bool operator==(const WebString& a, const WebString& b) {
  return a.Equals(b);
}

inline bool operator!=(const WebString& a, const WebString& b) {
  return !(a == b);
}

}  // namespace blink

#endif

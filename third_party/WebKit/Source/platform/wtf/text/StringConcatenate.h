/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef StringConcatenate_h
#define StringConcatenate_h

#include "platform/wtf/Allocator.h"
#include <string.h>

#ifndef WTFString_h
#include "platform/wtf/text/AtomicString.h"
#endif

// This macro is helpful for testing how many intermediate Strings are created
// while evaluating an expression containing operator+.
#ifndef WTF_STRINGTYPEADAPTER_COPIED_WTF_STRING
#define WTF_STRINGTYPEADAPTER_COPIED_WTF_STRING() ((void)0)
#endif

namespace WTF {

template <typename StringType>
class StringTypeAdapter {
  DISALLOW_NEW();
};

template <>
class StringTypeAdapter<char> {
  DISALLOW_NEW();

 public:
  explicit StringTypeAdapter<char>(char buffer) : buffer_(buffer) {}

  unsigned length() const { return 1; }
  bool Is8Bit() const { return true; }

  void WriteTo(LChar* destination) const { *destination = buffer_; }
  void WriteTo(UChar* destination) const { *destination = buffer_; }

 private:
  const unsigned char buffer_;
};

template <>
class StringTypeAdapter<LChar> {
  DISALLOW_NEW();

 public:
  explicit StringTypeAdapter<LChar>(LChar buffer) : buffer_(buffer) {}

  unsigned length() const { return 1; }
  bool Is8Bit() const { return true; }

  void WriteTo(LChar* destination) const { *destination = buffer_; }
  void WriteTo(UChar* destination) const { *destination = buffer_; }

 private:
  const LChar buffer_;
};

template <>
class StringTypeAdapter<UChar> {
  DISALLOW_NEW();

 public:
  explicit StringTypeAdapter<UChar>(UChar buffer) : buffer_(buffer) {}

  unsigned length() const { return 1; }
  bool Is8Bit() const { return buffer_ <= 0xff; }

  void WriteTo(LChar* destination) const {
    DCHECK(Is8Bit());
    *destination = static_cast<LChar>(buffer_);
  }

  void WriteTo(UChar* destination) const { *destination = buffer_; }

 private:
  const UChar buffer_;
};

template <>
class WTF_EXPORT StringTypeAdapter<char*> {
  DISALLOW_NEW();

 public:
  explicit StringTypeAdapter<char*>(char* buffer)
      : buffer_(buffer), length_(strlen(buffer)) {}

  unsigned length() const { return length_; }
  bool Is8Bit() const { return true; }

  void WriteTo(LChar* destination) const;
  void WriteTo(UChar* destination) const;

 private:
  const char* buffer_;
  unsigned length_;
};

template <>
class WTF_EXPORT StringTypeAdapter<LChar*> {
  DISALLOW_NEW();

 public:
  explicit StringTypeAdapter<LChar*>(LChar* buffer);

  unsigned length() const { return length_; }
  bool Is8Bit() const { return true; }

  void WriteTo(LChar* destination) const;
  void WriteTo(UChar* destination) const;

 private:
  const LChar* buffer_;
  const unsigned length_;
};

template <>
class WTF_EXPORT StringTypeAdapter<const UChar*> {
  DISALLOW_NEW();

 public:
  explicit StringTypeAdapter(const UChar* buffer);

  unsigned length() const { return length_; }
  bool Is8Bit() const { return false; }

  void WriteTo(LChar*) const { CHECK(false); }
  void WriteTo(UChar* destination) const;

 private:
  const UChar* buffer_;
  const unsigned length_;
};

template <>
class WTF_EXPORT StringTypeAdapter<const char*> {
  DISALLOW_NEW();

 public:
  explicit StringTypeAdapter<const char*>(const char* buffer);

  unsigned length() const { return length_; }
  bool Is8Bit() const { return true; }

  void WriteTo(LChar* destination) const;
  void WriteTo(UChar* destination) const;

 private:
  const char* buffer_;
  const unsigned length_;
};

template <>
class WTF_EXPORT StringTypeAdapter<const LChar*> {
  DISALLOW_NEW();

 public:
  explicit StringTypeAdapter<const LChar*>(const LChar* buffer);

  unsigned length() const { return length_; }
  bool Is8Bit() const { return true; }

  void WriteTo(LChar* destination) const;
  void WriteTo(UChar* destination) const;

 private:
  const LChar* buffer_;
  const unsigned length_;
};

template <>
class WTF_EXPORT StringTypeAdapter<StringView> {
  DISALLOW_NEW();

 public:
  explicit StringTypeAdapter(const StringView& view) : view_(view) {}

  unsigned length() const { return view_.length(); }
  bool Is8Bit() const { return view_.Is8Bit(); }

  void WriteTo(LChar* destination) const;
  void WriteTo(UChar* destination) const;

 private:
  const StringView view_;
};

template <>
class StringTypeAdapter<String> : public StringTypeAdapter<StringView> {
 public:
  explicit StringTypeAdapter(const String& string)
      : StringTypeAdapter<StringView>(string) {}
};

template <>
class StringTypeAdapter<AtomicString> : public StringTypeAdapter<StringView> {
 public:
  explicit StringTypeAdapter(const AtomicString& string)
      : StringTypeAdapter<StringView>(string) {}
};

}  // namespace WTF

#include "platform/wtf/text/StringOperators.h"
#endif

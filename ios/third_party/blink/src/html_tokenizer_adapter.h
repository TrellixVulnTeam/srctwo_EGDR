// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_THIRD_PARTY_BLINK_SRC_TOKENIZER_ADAPTER_H_
#define IOS_THIRD_PARTY_BLINK_SRC_TOKENIZER_ADAPTER_H_

#include <stddef.h>
#include <stdint.h>

#include "base/compiler_specific.h"
#include "base/logging.h"
#include "base/macros.h"

#define DEFINE_STATIC_LOCAL_STRING(name, arguments)                       \
    static const WebCore::LChar* name = (const WebCore::LChar*)arguments; \
    static const size_t name##Length = (arraysize(arguments) - 1); \
    DCHECK(name##Length == strlen((const char*)name))

#define WTF_MAKE_NONCOPYABLE(x) DISALLOW_COPY_AND_ASSIGN(x)

#define ASSERT(x) DCHECK(x)
#define ASSERT_NOT_REACHED NOTREACHED

#define notImplemented()

namespace WebCore {
typedef uint16_t UChar;
typedef uint8_t LChar;

template <typename CharType>
inline bool isASCIIUpper(CharType c) {
  return c >= 'A' && c <= 'Z';
}

template <typename CharType>
inline bool isASCIILower(CharType c) {
  return c >= 'a' && c <= 'z';
}

template <typename CharType>
inline CharType toLowerCase(CharType c) {
  ASSERT(isASCIIUpper(c));
  const int lowerCaseOffset = 0x20;
  return c + lowerCaseOffset;
}

inline UChar ByteSwap(UChar c) {
  return ((c & 0x00ff) << 8) | ((c & 0xff00) >> 8);
}
}

#endif  // IOS_THIRD_PARTY_BLINK_SRC_TOKENIZER_ADAPTER_H_

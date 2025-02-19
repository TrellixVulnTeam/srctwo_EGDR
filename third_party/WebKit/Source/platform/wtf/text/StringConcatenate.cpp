/*
 * Copyright 2014 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "platform/wtf/text/StringConcatenate.h"

#include "platform/wtf/text/StringImpl.h"

// This macro is helpful for testing how many intermediate Strings are created
// while evaluating an expression containing operator+.
#ifndef WTF_STRINGTYPEADAPTER_COPIED_WTF_STRING
#define WTF_STRINGTYPEADAPTER_COPIED_WTF_STRING() ((void)0)
#endif

void WTF::StringTypeAdapter<char*>::WriteTo(LChar* destination) const {
  for (unsigned i = 0; i < length_; ++i)
    destination[i] = static_cast<LChar>(buffer_[i]);
}

void WTF::StringTypeAdapter<char*>::WriteTo(UChar* destination) const {
  for (unsigned i = 0; i < length_; ++i) {
    unsigned char c = buffer_[i];
    destination[i] = c;
  }
}

WTF::StringTypeAdapter<LChar*>::StringTypeAdapter(LChar* buffer)
    : buffer_(buffer), length_(strlen(reinterpret_cast<char*>(buffer))) {}

void WTF::StringTypeAdapter<LChar*>::WriteTo(LChar* destination) const {
  memcpy(destination, buffer_, length_ * sizeof(LChar));
}

void WTF::StringTypeAdapter<LChar*>::WriteTo(UChar* destination) const {
  StringImpl::CopyChars(destination, buffer_, length_);
}

WTF::StringTypeAdapter<const UChar*>::StringTypeAdapter(const UChar* buffer)
    : buffer_(buffer), length_(LengthOfNullTerminatedString(buffer)) {}

void WTF::StringTypeAdapter<const UChar*>::WriteTo(UChar* destination) const {
  memcpy(destination, buffer_, length_ * sizeof(UChar));
}

WTF::StringTypeAdapter<const char*>::StringTypeAdapter(const char* buffer)
    : buffer_(buffer), length_(strlen(buffer)) {}

void WTF::StringTypeAdapter<const char*>::WriteTo(LChar* destination) const {
  memcpy(destination, buffer_, static_cast<size_t>(length_) * sizeof(LChar));
}

void WTF::StringTypeAdapter<const char*>::WriteTo(UChar* destination) const {
  for (unsigned i = 0; i < length_; ++i) {
    unsigned char c = buffer_[i];
    destination[i] = c;
  }
}

WTF::StringTypeAdapter<const LChar*>::StringTypeAdapter(const LChar* buffer)
    : buffer_(buffer), length_(strlen(reinterpret_cast<const char*>(buffer))) {}

void WTF::StringTypeAdapter<const LChar*>::WriteTo(LChar* destination) const {
  memcpy(destination, buffer_, static_cast<size_t>(length_) * sizeof(LChar));
}

void WTF::StringTypeAdapter<const LChar*>::WriteTo(UChar* destination) const {
  StringImpl::CopyChars(destination, buffer_, length_);
}

void WTF::StringTypeAdapter<StringView>::WriteTo(LChar* destination) const {
  DCHECK(Is8Bit());
  StringImpl::CopyChars(destination, view_.Characters8(), view_.length());
  WTF_STRINGTYPEADAPTER_COPIED_WTF_STRING();
}

void WTF::StringTypeAdapter<StringView>::WriteTo(UChar* destination) const {
  if (Is8Bit())
    StringImpl::CopyChars(destination, view_.Characters8(), view_.length());
  else
    StringImpl::CopyChars(destination, view_.Characters16(), view_.length());
  WTF_STRINGTYPEADAPTER_COPIED_WTF_STRING();
}

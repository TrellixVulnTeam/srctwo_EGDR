/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Google Inc. All rights reserved.
 * Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef Uint8ClampedArray_h
#define Uint8ClampedArray_h

#include "platform/wtf/MathExtras.h"
#include "platform/wtf/typed_arrays/Uint8Array.h"

namespace WTF {

class Uint8ClampedArray final : public Uint8Array {
 public:
  static inline RefPtr<Uint8ClampedArray> Create(unsigned length);
  static inline RefPtr<Uint8ClampedArray> Create(const unsigned char* array,
                                                 unsigned length);
  static inline RefPtr<Uint8ClampedArray> Create(RefPtr<ArrayBuffer>,
                                                 unsigned byte_offset,
                                                 unsigned length);

  using TypedArrayBase<unsigned char>::Set;
  inline void Set(unsigned index, double value);

  ViewType GetType() const override { return kTypeUint8Clamped; }

 private:
  inline Uint8ClampedArray(RefPtr<ArrayBuffer>,
                           unsigned byte_offset,
                           unsigned length);
  // Make constructor visible to superclass.
  friend class TypedArrayBase<unsigned char>;
};

RefPtr<Uint8ClampedArray> Uint8ClampedArray::Create(unsigned length) {
  return TypedArrayBase<unsigned char>::Create<Uint8ClampedArray>(length);
}

RefPtr<Uint8ClampedArray> Uint8ClampedArray::Create(const unsigned char* array,
                                                    unsigned length) {
  return TypedArrayBase<unsigned char>::Create<Uint8ClampedArray>(array,
                                                                  length);
}

RefPtr<Uint8ClampedArray> Uint8ClampedArray::Create(RefPtr<ArrayBuffer> buffer,
                                                    unsigned byte_offset,
                                                    unsigned length) {
  return TypedArrayBase<unsigned char>::Create<Uint8ClampedArray>(
      std::move(buffer), byte_offset, length);
}

void Uint8ClampedArray::Set(unsigned index, double value) {
  if (index >= length_)
    return;
  if (std::isnan(value) || value < 0)
    value = 0;
  else if (value > 255)
    value = 255;
  Data()[index] = static_cast<unsigned char>(lrint(value));
}

Uint8ClampedArray::Uint8ClampedArray(RefPtr<ArrayBuffer> buffer,
                                     unsigned byte_offset,
                                     unsigned length)
    : Uint8Array(std::move(buffer), byte_offset, length) {}

}  // namespace WTF

using WTF::Uint8ClampedArray;

#endif  // Uint8ClampedArray_h

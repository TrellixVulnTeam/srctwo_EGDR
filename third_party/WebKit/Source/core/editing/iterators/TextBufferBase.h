// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TextBufferBase_h
#define TextBufferBase_h

#include "core/CoreExport.h"
#include "platform/wtf/Vector.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class CORE_EXPORT TextBufferBase {
  STACK_ALLOCATED();
  WTF_MAKE_NONCOPYABLE(TextBufferBase);

 public:
  void Clear() { size_ = 0; }
  size_t Size() const { return size_; }
  bool IsEmpty() const { return size_ == 0; }
  size_t Capacity() const { return buffer_.capacity(); }
  const UChar& operator[](size_t index) const {
    DCHECK_LT(index, size_);
    return Data()[index];
  }
  virtual const UChar* Data() const = 0;

  void PushCharacters(UChar, size_t length);

  template <typename T>
  void PushRange(const T* other, size_t length) {
    if (length == 0)
      return;
    std::copy(other, other + length, EnsureDestination(length));
  }

  void Shrink(size_t delta) {
    DCHECK_LE(delta, size_);
    size_ -= delta;
  }

 protected:
  TextBufferBase();
  UChar* EnsureDestination(size_t length);
  void Grow(size_t demand);

  virtual UChar* CalcDestination(size_t length) = 0;
  virtual void ShiftData(size_t old_capacity);

  const UChar* BufferBegin() const { return buffer_.begin(); }
  const UChar* BufferEnd() const { return buffer_.end(); }
  UChar* BufferBegin() { return buffer_.begin(); }
  UChar* BufferEnd() { return buffer_.end(); }

 private:
  size_t size_ = 0;
  Vector<UChar, 1024> buffer_;
};

}  // namespace blink

#endif  // TextBufferBase_h

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_BASE_RING_BUFFER_H_
#define CC_BASE_RING_BUFFER_H_

#include <stddef.h>

#include "base/logging.h"
#include "base/macros.h"

namespace cc {

template <typename T, size_t kSize>
class RingBuffer {
 public:
  RingBuffer() : current_index_(0) {}

  size_t BufferSize() const { return kSize; }

  size_t CurrentIndex() const { return current_index_; }

  // tests if a value was saved to this index
  bool IsFilledIndex(size_t n) const { return BufferIndex(n) < current_index_; }

  // n = 0 returns the oldest value and
  // n = bufferSize() - 1 returns the most recent value.
  const T& ReadBuffer(size_t n) const {
    DCHECK(IsFilledIndex(n));
    return buffer_[BufferIndex(n)];
  }

  T* MutableReadBuffer(size_t n) {
    DCHECK(IsFilledIndex(n));
    return &buffer_[BufferIndex(n)];
  }

  void SaveToBuffer(const T& value) {
    buffer_[BufferIndex(0)] = value;
    current_index_++;
  }

  void Clear() { current_index_ = 0; }

  // Iterator has const access to the RingBuffer it got retrieved from.
  class Iterator {
   public:
    size_t index() const { return index_; }

    const T* operator->() const { return &buffer_.ReadBuffer(index_); }
    const T* operator*() const { return &buffer_.ReadBuffer(index_); }

    Iterator& operator++() {
      index_++;
      if (index_ == kSize)
        out_of_range_ = true;
      return *this;
    }

    Iterator& operator--() {
      if (index_ == 0)
        out_of_range_ = true;
      index_--;
      return *this;
    }

    operator bool() const {
      return buffer_.IsFilledIndex(index_) && !out_of_range_;
    }

   private:
    Iterator(const RingBuffer<T, kSize>& buffer, size_t index)
        : buffer_(buffer), index_(index), out_of_range_(false) {}

    const RingBuffer<T, kSize>& buffer_;
    size_t index_;
    bool out_of_range_;

    friend class RingBuffer<T, kSize>;
  };

  // Returns an Iterator pointing to the oldest value in the buffer.
  // Example usage (iterate from oldest to newest value):
  //  for (RingBuffer<T, kSize>::Iterator it = ring_buffer.Begin(); it; ++it) {}
  Iterator Begin() const {
    if (current_index_ < kSize)
      return Iterator(*this, kSize - current_index_);
    return Iterator(*this, 0);
  }

  // Returns an Iterator pointing to the newest value in the buffer.
  // Example usage (iterate backwards from newest to oldest value):
  //  for (RingBuffer<T, kSize>::Iterator it = ring_buffer.End(); it; --it) {}
  Iterator End() const { return Iterator(*this, kSize - 1); }

 private:
  inline size_t BufferIndex(size_t n) const {
    return (current_index_ + n) % kSize;
  }

  T buffer_[kSize];
  size_t current_index_;

  DISALLOW_COPY_AND_ASSIGN(RingBuffer);
};

}  // namespace cc

#endif  // CC_BASE_RING_BUFFER_H_

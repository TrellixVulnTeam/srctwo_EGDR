// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef TerminatedArray_h
#define TerminatedArray_h

#include "platform/wtf/Allocator.h"
#include "platform/wtf/PtrUtil.h"
#include "platform/wtf/VectorTraits.h"
#include "platform/wtf/allocator/Partitions.h"
#include <memory>

namespace WTF {

// TerminatedArray<T> represents a sequence of elements of type T in which each
// element knows whether it is the last element in the sequence or not. For this
// check type T must provide isLastInArray method.
// TerminatedArray<T> can only be constructed by TerminatedArrayBuilder<T>.
template <typename T>
class TerminatedArray {
  DISALLOW_NEW();
  WTF_MAKE_NONCOPYABLE(TerminatedArray);

 public:
  // When TerminatedArray::Allocator implementations grow the backing
  // store, old is copied into the new and larger block.
  static_assert(VectorTraits<T>::kCanCopyWithMemcpy,
                "Array elements must be memory copyable");

  T& at(size_t index) { return reinterpret_cast<T*>(this)[index]; }
  const T& at(size_t index) const {
    return reinterpret_cast<const T*>(this)[index];
  }

  template <typename U>
  class iterator_base final {
    STACK_ALLOCATED();

   public:
    iterator_base& operator++() {
      if (val_->IsLastInArray()) {
        val_ = 0;
      } else {
        ++val_;
      }
      return *this;
    }

    U& operator*() const { return *val_; }

    bool operator==(const iterator_base& other) const {
      return val_ == other.val_;
    }
    bool operator!=(const iterator_base& other) const {
      return !(*this == other);
    }

   private:
    iterator_base(U* val) : val_(val) {}

    U* val_;

    friend class TerminatedArray;
  };

  typedef iterator_base<T> iterator;
  typedef iterator_base<const T> const_iterator;

  iterator begin() { return iterator(reinterpret_cast<T*>(this)); }
  const_iterator begin() const {
    return const_iterator(reinterpret_cast<const T*>(this));
  }

  iterator end() { return iterator(0); }
  const_iterator end() const { return const_iterator(0); }

  size_t size() const {
    size_t count = 0;
    for (const_iterator it = begin(); it != end(); ++it)
      count++;
    return count;
  }

  // Match Allocator semantics to be able to use
  // std::unique_ptr<TerminatedArray>.
  void operator delete(void* p) { ::WTF::Partitions::FastFree(p); }

 private:
  // Allocator describes how TerminatedArrayBuilder should create new instances
  // of TerminateArray and manage their lifetimes.
  struct Allocator {
    STATIC_ONLY(Allocator);
    using PassPtr = std::unique_ptr<TerminatedArray>;
    using Ptr = std::unique_ptr<TerminatedArray>;

    static PassPtr Release(Ptr& ptr) { return ptr.release(); }

    static PassPtr Create(size_t capacity) {
      return WTF::WrapUnique(
          static_cast<TerminatedArray*>(WTF::Partitions::FastMalloc(
              WTF::Partitions::ComputeAllocationSize(capacity, sizeof(T)),
              WTF_HEAP_PROFILER_TYPE_NAME(T))));
    }

    static PassPtr Resize(Ptr ptr, size_t capacity) {
      return WTF::WrapUnique(
          static_cast<TerminatedArray*>(WTF::Partitions::FastRealloc(
              ptr.release(),
              WTF::Partitions::ComputeAllocationSize(capacity, sizeof(T)),
              WTF_HEAP_PROFILER_TYPE_NAME(T))));
    }
  };

  // Prohibit construction. Allocator makes TerminatedArray instances for
  // TerminatedArrayBuilder by pointer casting.
  TerminatedArray();

  template <typename, template <typename> class>
  friend class TerminatedArrayBuilder;
};

}  // namespace WTF

using WTF::TerminatedArray;

#endif  // TerminatedArray_h

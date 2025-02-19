// Copyright 2015 The Crashpad Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef CRASHPAD_UTIL_STDLIB_ALIGNED_ALLOCATOR_H_
#define CRASHPAD_UTIL_STDLIB_ALIGNED_ALLOCATOR_H_

#include <stddef.h>

#include <limits>
#include <memory>
#include <new>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "util/stdlib/cxx.h"

#if defined(COMPILER_MSVC) && _MSC_VER < 1900
#define CRASHPAD_NOEXCEPT _NOEXCEPT
#else
#define CRASHPAD_NOEXCEPT noexcept
#endif

namespace crashpad {
namespace internal {

//! \brief Allocates memory with the specified alignment constraint.
//!
//! This function wraps `posix_memalign()` or `_aligned_malloc()`. Memory
//! allocated by this function must be released by AlignFree().
void* AlignedAllocate(size_t alignment, size_t size);

//! \brief Frees memory allocated by AlignedAllocate().
//!
//! This function wraps `free()` or `_aligned_free()`.
void AlignedFree(void* pointer);

}  // namespace internal

//! \brief A standard allocator that aligns its allocations as requested,
//!     suitable for use as an allocator in standard containers.
//!
//! This is similar to `std::allocator<T>`, with the addition of an alignment
//! guarantee. \a Alignment must be a power of 2. If \a Alignment is not
//! specified, the default alignment for type \a T is used.
template <class T, size_t Alignment = alignof(T)>
struct AlignedAllocator {
 public:
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  template <class U>
  struct rebind {
    using other = AlignedAllocator<U, Alignment>;
  };

  AlignedAllocator() CRASHPAD_NOEXCEPT {}
  AlignedAllocator(const AlignedAllocator& other) CRASHPAD_NOEXCEPT {}

  template <typename U>
  AlignedAllocator(const AlignedAllocator<U, Alignment>& other)
      CRASHPAD_NOEXCEPT {}

  ~AlignedAllocator() {}

  pointer address(reference x) const CRASHPAD_NOEXCEPT { return &x; }
  const_pointer address(const_reference x) const CRASHPAD_NOEXCEPT {
    return &x;
  }

  pointer allocate(size_type n, std::allocator<void>::const_pointer hint = 0) {
    return reinterpret_cast<pointer>(
        internal::AlignedAllocate(Alignment, sizeof(value_type) * n));
  }

  void deallocate(pointer p, size_type n) { internal::AlignedFree(p); }

  size_type max_size() const CRASHPAD_NOEXCEPT {
    return std::numeric_limits<size_type>::max() / sizeof(value_type);
  }

#if CXX_LIBRARY_VERSION < 2011
  void construct(pointer p, const T& val) {
    new (reinterpret_cast<void*>(p)) T(val);
  }
#else
  template <class U, class... Args>
  void construct(U* p, Args&&... args) {
    new (reinterpret_cast<void*>(p)) U(std::forward<Args>(args)...);
  }
#endif

  template <class U>
  void destroy(U* p) {
    p->~U();
  }
};

template <class T1, class T2, size_t Alignment>
bool operator==(const AlignedAllocator<T1, Alignment>& lhs,
                const AlignedAllocator<T2, Alignment>& rhs) CRASHPAD_NOEXCEPT {
  return true;
}

template <class T1, class T2, size_t Alignment>
bool operator!=(const AlignedAllocator<T1, Alignment>& lhs,
                const AlignedAllocator<T2, Alignment>& rhs) CRASHPAD_NOEXCEPT {
  return false;
}

//! \brief A `std::vector` using AlignedAllocator.
//!
//! This is similar to `std::vector<T>`, with the addition of an alignment
//! guarantee. \a Alignment must be a power of 2. If \a Alignment is not
//! specified, the default alignment for type \a T is used.
template <typename T, size_t Alignment = alignof(T)>
using AlignedVector = std::vector<T, AlignedAllocator<T, Alignment>>;

}  // namespace crashpad

#undef CRASHPAD_NOEXCEPT

#endif  // CRASHPAD_UTIL_STDLIB_ALIGNED_ALLOCATOR_H_

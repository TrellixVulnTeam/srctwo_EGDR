// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MEMORY_PTR_UTIL_H_
#define BASE_MEMORY_PTR_UTIL_H_

#include <memory>
#include <utility>

namespace base {

// Helper to transfer ownership of a raw pointer to a std::unique_ptr<T>.
// Note that std::unique_ptr<T> has very different semantics from
// std::unique_ptr<T[]>: do not use this helper for array allocations.
template <typename T>
std::unique_ptr<T> WrapUnique(T* ptr) {
  return std::unique_ptr<T>(ptr);
}

// TODO(crbug.com/755727): Inline all uses.
template <typename T, typename... Args>
auto MakeUnique(Args&&... args)
    -> decltype(std::make_unique<T>(std::forward<Args>(args)...)) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

}  // namespace base

#endif  // BASE_MEMORY_PTR_UTIL_H_

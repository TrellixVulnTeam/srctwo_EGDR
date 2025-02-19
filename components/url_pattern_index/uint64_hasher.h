// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Based on "v8/src/base/functional.cc".
// See: Thomas Wang, Integer Hash Functions.
// https://gist.github.com/badboy/6267743
// TODO(pkalinnikov): Consider moving the implementation into base/.

#ifndef COMPONENTS_URL_PATTERN_INDEX_HASH_H_
#define COMPONENTS_URL_PATTERN_INDEX_HASH_H_

#include <stddef.h>
#include <stdint.h>

#include <type_traits>

namespace url_pattern_index {

template <typename T>
typename std::enable_if<sizeof(T) == 4, T>::type Uint64Hash(uint64_t v) {
  // "64 bit to 32 bit Hash Functions"
  v = ~v + (v << 18);  // v = (v << 18) - v - 1;
  v = v ^ (v >> 31);
  v = v * 21;  // v = (v + (v << 2)) + (v << 4);
  v = v ^ (v >> 11);
  v = v + (v << 6);
  v = v ^ (v >> 22);
  return static_cast<T>(v);
}

template <typename T>
typename std::enable_if<sizeof(T) == 8, T>::type Uint64Hash(uint64_t v) {
  // "64 bit Mix Functions"
  v = ~v + (v << 21);  // v = (v << 21) - v - 1;
  v = v ^ (v >> 24);
  v = (v + (v << 3)) + (v << 8);  // v * 265
  v = v ^ (v >> 14);
  v = (v + (v << 2)) + (v << 4);  // v * 21
  v = v ^ (v >> 28);
  v = v + (v << 31);
  return static_cast<T>(v);
}

class Uint64Hasher {
 public:
  size_t operator()(uint64_t v) const { return Uint64Hash<size_t>(v); }
};

}  // namespace url_pattern_index

#endif  // COMPONENTS_URL_PATTERN_INDEX_HASH_H_

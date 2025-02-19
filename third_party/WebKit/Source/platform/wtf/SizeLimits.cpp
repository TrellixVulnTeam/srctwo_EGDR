/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "platform/wtf/Assertions.h"
#include "platform/wtf/ContainerAnnotations.h"
#include "platform/wtf/PassRefPtr.h"
#include "platform/wtf/RefCounted.h"
#include "platform/wtf/RefPtr.h"
#include "platform/wtf/ThreadRestrictionVerifier.h"
#include "platform/wtf/Vector.h"
#include "platform/wtf/text/AtomicString.h"
#include "platform/wtf/text/WTFString.h"
#include <memory>

namespace WTF {

#if DCHECK_IS_ON() || ENABLE_SECURITY_ASSERT
// The debug/assertion version may get bigger.
struct SameSizeAsRefCounted {
  int a;
#if ENABLE_SECURITY_ASSERT
  bool b;
#endif
#if DCHECK_IS_ON()
  bool c;
  ThreadRestrictionVerifier d;
#endif
};
#else
struct SameSizeAsRefCounted {
  int a;
  // Don't add anything here because this should stay small.
};
#endif
template <typename T, unsigned inlineCapacity = 0>
struct SameSizeAsVectorWithInlineCapacity;

template <typename T>
struct SameSizeAsVectorWithInlineCapacity<T, 0> {
  void* buffer_pointer;
  unsigned capacity;
  unsigned size;
};

template <typename T, unsigned inlineCapacity>
struct SameSizeAsVectorWithInlineCapacity {
  SameSizeAsVectorWithInlineCapacity<T, 0> base_capacity;
#if !defined(ANNOTATE_CONTIGUOUS_CONTAINER)
  AlignedBuffer<inlineCapacity * sizeof(T), WTF_ALIGN_OF(T)> inline_buffer;
#endif
};

static_assert(sizeof(std::unique_ptr<int>) == sizeof(int*),
              "std::unique_ptr should stay small");
static_assert(sizeof(PassRefPtr<RefCounted<int>>) == sizeof(int*),
              "PassRefPtr should stay small");
static_assert(sizeof(RefCounted<int>) == sizeof(SameSizeAsRefCounted),
              "RefCounted should stay small");
static_assert(sizeof(RefPtr<RefCounted<int>>) == sizeof(int*),
              "RefPtr should stay small");
static_assert(sizeof(String) == sizeof(int*), "String should stay small");
static_assert(sizeof(AtomicString) == sizeof(String),
              "AtomicString should stay small");
static_assert(sizeof(Vector<int>) ==
                  sizeof(SameSizeAsVectorWithInlineCapacity<int>),
              "Vector should stay small");
static_assert(sizeof(Vector<int, 1>) ==
                  sizeof(SameSizeAsVectorWithInlineCapacity<int, 1>),
              "Vector should stay small");
static_assert(sizeof(Vector<int, 2>) ==
                  sizeof(SameSizeAsVectorWithInlineCapacity<int, 2>),
              "Vector should stay small");
static_assert(sizeof(Vector<int, 3>) ==
                  sizeof(SameSizeAsVectorWithInlineCapacity<int, 3>),
              "Vector should stay small");
}  // namespace WTF

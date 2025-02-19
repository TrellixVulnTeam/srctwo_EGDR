/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ArenaTestHelpers_h
#define ArenaTestHelpers_h

#include "platform/PODArena.h"
#include "platform/wtf/NotFound.h"
#include "platform/wtf/Vector.h"

#include <gtest/gtest.h>

namespace blink {
namespace ArenaTestHelpers {

// An allocator for the PODArena which tracks the regions which have
// been allocated.
class TrackedAllocator final : public PODArena::FastMallocAllocator {
 public:
  static PassRefPtr<TrackedAllocator> Create() {
    return AdoptRef(new TrackedAllocator);
  }

  void* Allocate(size_t size) override {
    void* result = PODArena::FastMallocAllocator::Allocate(size);
    allocated_regions_.push_back(result);
    return result;
  }

  void Free(void* ptr) override {
    size_t slot = allocated_regions_.Find(ptr);
    ASSERT_NE(slot, kNotFound);
    allocated_regions_.erase(slot);
    PODArena::FastMallocAllocator::Free(ptr);
  }

  bool IsEmpty() const { return !NumRegions(); }

  int NumRegions() const { return allocated_regions_.size(); }

 private:
  TrackedAllocator() {}
  Vector<void*> allocated_regions_;
};

}  // namespace ArenaTestHelpers
}  // namespace blink

#endif  // ArenaTestHelpers_h

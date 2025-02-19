// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/heap/HeapAllocator.h"

namespace blink {

void HeapAllocator::BackingFree(void* address) {
  if (!address)
    return;

  ThreadState* state = ThreadState::Current();
  if (state->SweepForbidden())
    return;
  DCHECK(!state->IsInGC());

  // Don't promptly free large objects because their page is never reused.
  // Don't free backings allocated on other threads.
  BasePage* page = PageFromObject(address);
  if (page->IsLargeObjectPage() || page->Arena()->GetThreadState() != state)
    return;

  HeapObjectHeader* header = HeapObjectHeader::FromPayload(address);
  NormalPageArena* arena = static_cast<NormalPage*>(page)->ArenaForNormalPage();
  state->PromptlyFreed(header->GcInfoIndex());
  arena->PromptlyFreeObject(header);
}

void HeapAllocator::FreeVectorBacking(void* address) {
  BackingFree(address);
}

void HeapAllocator::FreeInlineVectorBacking(void* address) {
  BackingFree(address);
}

void HeapAllocator::FreeHashTableBacking(void* address) {
  BackingFree(address);
}

bool HeapAllocator::BackingExpand(void* address, size_t new_size) {
  if (!address)
    return false;

  ThreadState* state = ThreadState::Current();
  if (state->SweepForbidden())
    return false;
  DCHECK(!state->IsInGC());
  DCHECK(state->IsAllocationAllowed());
  DCHECK_EQ(&state->Heap(), &ThreadState::FromObject(address)->Heap());

  // FIXME: Support expand for large objects.
  // Don't expand backings allocated on other threads.
  BasePage* page = PageFromObject(address);
  if (page->IsLargeObjectPage() || page->Arena()->GetThreadState() != state)
    return false;

  HeapObjectHeader* header = HeapObjectHeader::FromPayload(address);
  NormalPageArena* arena = static_cast<NormalPage*>(page)->ArenaForNormalPage();
  bool succeed = arena->ExpandObject(header, new_size);
  if (succeed)
    state->AllocationPointAdjusted(arena->ArenaIndex());
  return succeed;
}

bool HeapAllocator::ExpandVectorBacking(void* address, size_t new_size) {
  return BackingExpand(address, new_size);
}

bool HeapAllocator::ExpandInlineVectorBacking(void* address, size_t new_size) {
  return BackingExpand(address, new_size);
}

bool HeapAllocator::ExpandHashTableBacking(void* address, size_t new_size) {
  return BackingExpand(address, new_size);
}

bool HeapAllocator::BackingShrink(void* address,
                                  size_t quantized_current_size,
                                  size_t quantized_shrunk_size) {
  if (!address || quantized_shrunk_size == quantized_current_size)
    return true;

  DCHECK_LT(quantized_shrunk_size, quantized_current_size);

  ThreadState* state = ThreadState::Current();
  if (state->SweepForbidden())
    return false;
  DCHECK(!state->IsInGC());
  DCHECK(state->IsAllocationAllowed());
  DCHECK_EQ(&state->Heap(), &ThreadState::FromObject(address)->Heap());

  // FIXME: Support shrink for large objects.
  // Don't shrink backings allocated on other threads.
  BasePage* page = PageFromObject(address);
  if (page->IsLargeObjectPage() || page->Arena()->GetThreadState() != state)
    return false;

  HeapObjectHeader* header = HeapObjectHeader::FromPayload(address);
  NormalPageArena* arena = static_cast<NormalPage*>(page)->ArenaForNormalPage();
  // We shrink the object only if the shrinking will make a non-small
  // prompt-free block.
  // FIXME: Optimize the threshold size.
  if (quantized_current_size <= quantized_shrunk_size +
                                    sizeof(HeapObjectHeader) +
                                    sizeof(void*) * 32 &&
      !arena->IsObjectAllocatedAtAllocationPoint(header))
    return true;

  bool succeeded_at_allocation_point =
      arena->ShrinkObject(header, quantized_shrunk_size);
  if (succeeded_at_allocation_point)
    state->AllocationPointAdjusted(arena->ArenaIndex());
  return true;
}

bool HeapAllocator::ShrinkVectorBacking(void* address,
                                        size_t quantized_current_size,
                                        size_t quantized_shrunk_size) {
  return BackingShrink(address, quantized_current_size, quantized_shrunk_size);
}

bool HeapAllocator::ShrinkInlineVectorBacking(void* address,
                                              size_t quantized_current_size,
                                              size_t quantized_shrunk_size) {
  return BackingShrink(address, quantized_current_size, quantized_shrunk_size);
}

}  // namespace blink

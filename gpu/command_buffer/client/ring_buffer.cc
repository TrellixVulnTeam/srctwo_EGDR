// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains the implementation of the RingBuffer class.

#include "gpu/command_buffer/client/ring_buffer.h"

#include <stdint.h>

#include <algorithm>

#include "base/logging.h"
#include "gpu/command_buffer/client/cmd_buffer_helper.h"

namespace gpu {

RingBuffer::RingBuffer(unsigned int alignment,
                       Offset base_offset,
                       unsigned int size,
                       CommandBufferHelper* helper,
                       void* base)
    : helper_(helper),
      base_offset_(base_offset),
      size_(size),
      free_offset_(0),
      in_use_offset_(0),
      alignment_(alignment),
      base_(static_cast<int8_t*>(base) - base_offset) {}

RingBuffer::~RingBuffer() {
  for (const auto& block : blocks_)
    DCHECK(block.state != IN_USE);
}

void RingBuffer::FreeOldestBlock() {
  DCHECK(!blocks_.empty()) << "no free blocks";
  Block& block = blocks_.front();
  DCHECK(block.state != IN_USE)
      << "attempt to allocate more than maximum memory";
  if (block.state == FREE_PENDING_TOKEN) {
    helper_->WaitForToken(block.token);
  }
  in_use_offset_ += block.size;
  if (in_use_offset_ == size_) {
    in_use_offset_ = 0;
  }
  // If they match then the entire buffer is free.
  if (in_use_offset_ == free_offset_) {
    in_use_offset_ = 0;
    free_offset_ = 0;
  }
  blocks_.pop_front();
}

void* RingBuffer::Alloc(unsigned int size) {
  DCHECK_LE(size, size_) << "attempt to allocate more than maximum memory";
  DCHECK(blocks_.empty() || blocks_.back().state != IN_USE)
      << "Attempt to alloc another block before freeing the previous.";
  // Similarly to malloc, an allocation of 0 allocates at least 1 byte, to
  // return different pointers every time.
  if (size == 0) size = 1;
  // Allocate rounded to alignment size so that the offsets are always
  // memory-aligned.
  size = RoundToAlignment(size);

  // Wait until there is enough room.
  while (size > GetLargestFreeSizeNoWaiting()) {
    FreeOldestBlock();
  }

  if (size + free_offset_ > size_) {
    // Add padding to fill space before wrapping around
    blocks_.push_back(Block(free_offset_, size_ - free_offset_, PADDING));
    free_offset_ = 0;
  }

  Offset offset = free_offset_;
  blocks_.push_back(Block(offset, size, IN_USE));
  free_offset_ += size;
  if (free_offset_ == size_) {
    free_offset_ = 0;
  }
  return GetPointer(offset + base_offset_);
}

void RingBuffer::FreePendingToken(void* pointer,
                                  unsigned int token) {
  Offset offset = GetOffset(pointer);
  offset -= base_offset_;
  DCHECK(!blocks_.empty()) << "no allocations to free";
  for (Container::reverse_iterator it = blocks_.rbegin();
        it != blocks_.rend();
        ++it) {
    Block& block = *it;
    if (block.offset == offset) {
      DCHECK(block.state == IN_USE)
          << "block that corresponds to offset already freed";
      block.token = token;
      block.state = FREE_PENDING_TOKEN;
      return;
    }
  }
  NOTREACHED() << "attempt to free non-existant block";
}

void RingBuffer::DiscardBlock(void* pointer) {
  Offset offset = GetOffset(pointer);
  offset -= base_offset_;
  DCHECK(!blocks_.empty()) << "no allocations to discard";
  for (Container::reverse_iterator it = blocks_.rbegin();
        it != blocks_.rend();
        ++it) {
    Block& block = *it;
    if (block.offset == offset) {
      DCHECK(block.state != PADDING)
          << "block that corresponds to offset already discarded";
      block.state = PADDING;

      // Remove block if it were in the back along with any extra padding.
      while (!blocks_.empty() && blocks_.back().state == PADDING) {
        free_offset_= blocks_.back().offset;
        blocks_.pop_back();
      }

      // Remove blocks if it were in the front along with extra padding.
      while (!blocks_.empty() && blocks_.front().state == PADDING) {
        blocks_.pop_front();
        if (blocks_.empty())
          break;

        in_use_offset_ = blocks_.front().offset;
      }

      // In the special case when there are no blocks, we should be reset it.
      if (blocks_.empty()) {
        in_use_offset_ = 0;
        free_offset_ = 0;
      }
      return;
    }
  }
  NOTREACHED() << "attempt to discard non-existant block";
}

unsigned int RingBuffer::GetLargestFreeSizeNoWaiting() {
  while (!blocks_.empty()) {
    Block& block = blocks_.front();
    if (!helper_->HasTokenPassed(block.token) || block.state == IN_USE) break;
    FreeOldestBlock();
  }
  if (free_offset_ == in_use_offset_) {
    if (blocks_.empty()) {
      // The entire buffer is free.
      DCHECK_EQ(free_offset_, 0u);
      return size_;
    } else {
      // The entire buffer is in use.
      return 0;
    }
  } else if (free_offset_ > in_use_offset_) {
    // It's free from free_offset_ to size_ and from 0 to in_use_offset_
    return std::max(size_ - free_offset_, in_use_offset_);
  } else {
    // It's free from free_offset_ -> in_use_offset_;
    return in_use_offset_ - free_offset_;
  }
}

unsigned int RingBuffer::GetTotalFreeSizeNoWaiting() {
  unsigned int largest_free_size = GetLargestFreeSizeNoWaiting();
  if (free_offset_ > in_use_offset_) {
    // It's free from free_offset_ to size_ and from 0 to in_use_offset_.
    return size_ - free_offset_ + in_use_offset_;
  } else {
    return largest_free_size;
  }
}

void RingBuffer::ShrinkLastBlock(unsigned int new_size) {
  if (blocks_.empty())
    return;
  auto& block = blocks_.back();
  DCHECK_LT(new_size, block.size);
  DCHECK_EQ(block.state, IN_USE);

  // Can't shrink to size 0, see comments in Alloc.
  new_size = std::max(new_size, 1u);

  free_offset_ = block.offset + new_size;
  block.size = new_size;
}

}  // namespace gpu

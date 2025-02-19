/*
 * Copyright (C) 2010 Google, Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY GOOGLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL GOOGLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef IgnoreDestructiveWriteCountIncrementer_h
#define IgnoreDestructiveWriteCountIncrementer_h

#include "core/dom/Document.h"
#include "platform/wtf/Allocator.h"

namespace blink {

class IgnoreDestructiveWriteCountIncrementer {
  STACK_ALLOCATED();
  WTF_MAKE_NONCOPYABLE(IgnoreDestructiveWriteCountIncrementer);

 public:
  explicit IgnoreDestructiveWriteCountIncrementer(Document* document)
      : count_(document ? &document->ignore_destructive_write_count_ : 0) {
    if (!count_)
      return;
    ++(*count_);
  }

  ~IgnoreDestructiveWriteCountIncrementer() {
    if (!count_)
      return;
    --(*count_);
  }

 private:
  unsigned* count_;
};

}  // namespace blink

#endif

/*
 * Copyright (C) 2009 Apple Inc.  All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LayoutObjectChildList_h
#define LayoutObjectChildList_h

#include "platform/heap/Handle.h"
#include "platform/wtf/Forward.h"

namespace blink {

class LayoutObject;

class LayoutObjectChildList {
  DISALLOW_NEW();

 public:
  LayoutObjectChildList() : first_child_(nullptr), last_child_(nullptr) {}

  LayoutObject* FirstChild() const { return first_child_; }
  LayoutObject* LastChild() const { return last_child_; }

  void DestroyLeftoverChildren();

  LayoutObject* RemoveChildNode(LayoutObject* owner,
                                LayoutObject*,
                                bool notify_layout_object = true);
  void InsertChildNode(LayoutObject* owner,
                       LayoutObject* new_child,
                       LayoutObject* before_child,
                       bool notify_layout_object = true);
  void AppendChildNode(LayoutObject* owner,
                       LayoutObject* new_child,
                       bool notify_layout_object = true) {
    InsertChildNode(owner, new_child, 0, notify_layout_object);
  }

 private:
  void InvalidatePaintOnRemoval(LayoutObject& old_child);

  LayoutObject* first_child_;
  LayoutObject* last_child_;
};

}  // namespace blink

#endif  // LayoutObjectChildList_h

/*
 * Copyright (C) 2007, 2012 Apple Inc.  All rights reserved.
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

#ifndef DragActions_h
#define DragActions_h

#include <limits.h>

namespace blink {

typedef enum {
  kDragDestinationActionNone = 0,
  kDragDestinationActionDHTML = 1,
  kDragDestinationActionEdit = 2,
  kDragDestinationActionLoad = 4,
  kDragDestinationActionAny = UINT_MAX
} DragDestinationAction;

typedef enum {
  kDragSourceActionNone,
  kDragSourceActionDHTML,
  kDragSourceActionImage,
  kDragSourceActionLink,
  kDragSourceActionSelection,
} DragSourceAction;

// matches NSDragOperation
typedef enum {
  kDragOperationNone = 0,
  kDragOperationCopy = 1,
  kDragOperationLink = 2,
  kDragOperationGeneric = 4,
  kDragOperationPrivate = 8,
  kDragOperationMove = 16,
  kDragOperationDelete = 32,
  kDragOperationEvery = UINT_MAX
} DragOperation;

}  // namespace blink

#endif  // !DragActions_h

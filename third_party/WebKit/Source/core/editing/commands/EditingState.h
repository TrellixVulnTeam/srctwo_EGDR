// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EditingState_h
#define EditingState_h

#include "core/CoreExport.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/Assertions.h"
#include "platform/wtf/Noncopyable.h"

namespace blink {

// EditingState represents current editing command running state for propagating
// DOM tree mutation operation failure to callers.
//
// Example usage:
//  EditingState editingState;
//  ...
//  functionMutatesDOMTree(..., &editingState);
//  if (editingState.isAborted())
//      return;
//
class CORE_EXPORT EditingState final {
  STACK_ALLOCATED();
  WTF_MAKE_NONCOPYABLE(EditingState);

 public:
  EditingState();
  ~EditingState();

  void Abort();
  bool IsAborted() const { return is_aborted_; }

 private:
  bool is_aborted_ = false;
};

// TODO(yosin): Once all commands aware |EditingState|, we get rid of
// |IgnorableEditingAbortState | class
class IgnorableEditingAbortState final {
  STACK_ALLOCATED();
  WTF_MAKE_NONCOPYABLE(IgnorableEditingAbortState);

 public:
  IgnorableEditingAbortState();
  ~IgnorableEditingAbortState();

  EditingState* GetEditingState() { return &editing_state_; }

 private:
  EditingState editing_state_;
};

// Abort the editing command if the specified expression is true.
#define ABORT_EDITING_COMMAND_IF(expr) \
  do {                                 \
    if (expr) {                        \
      editing_state->Abort();          \
      return;                          \
    }                                  \
  } while (false)

#if DCHECK_IS_ON()
// This class is inspired by |NoExceptionStateAssertionChecker|.
class NoEditingAbortChecker final {
  STACK_ALLOCATED();
  WTF_MAKE_NONCOPYABLE(NoEditingAbortChecker);

 public:
  NoEditingAbortChecker(const char* file, int line);
  ~NoEditingAbortChecker();

  EditingState* GetEditingState() { return &editing_state_; }

 private:
  EditingState editing_state_;
  const char* const file_;
  int const line_;
};

// If a function with EditingState* argument should not be aborted,
// ASSERT_NO_EDITING_ABORT should be specified.
//    fooFunc(...., ASSERT_NO_EDITING_ABORT);
// It causes an assertion failure If DCHECK_IS_ON() and the function was aborted
// unexpectedly.
#define ASSERT_NO_EDITING_ABORT \
  (NoEditingAbortChecker(__FILE__, __LINE__).GetEditingState())
#else
#define ASSERT_NO_EDITING_ABORT (IgnorableEditingAbortState().GetEditingState())
#endif

}  // namespace blink

#endif  // EditingState_h

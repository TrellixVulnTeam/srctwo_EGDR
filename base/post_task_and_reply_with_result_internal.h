// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_POST_TASK_AND_REPLY_WITH_RESULT_INTERNAL_H_
#define BASE_POST_TASK_AND_REPLY_WITH_RESULT_INTERNAL_H_

#include <utility>

#include "base/callback.h"

namespace base {

namespace internal {

// Adapts a function that produces a result via a return value to
// one that returns via an output parameter.
template <typename ReturnType>
void ReturnAsParamAdapter(OnceCallback<ReturnType()> func, ReturnType* result) {
  *result = std::move(func).Run();
}

// Adapts a T* result to a callblack that expects a T.
template <typename TaskReturnType, typename ReplyArgType>
void ReplyAdapter(OnceCallback<void(ReplyArgType)> callback,
                  TaskReturnType* result) {
  std::move(callback).Run(std::move(*result));
}

}  // namespace internal

}  // namespace base

#endif  // BASE_POST_TASK_AND_REPLY_WITH_RESULT_INTERNAL_H_

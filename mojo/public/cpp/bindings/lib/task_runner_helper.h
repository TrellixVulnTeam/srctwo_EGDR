// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_PUBLIC_CPP_BINDINGS_LIB_TASK_RUNNER_HELPER_H_
#define MOJO_PUBLIC_CPP_BINDINGS_LIB_TASK_RUNNER_HELPER_H_

#include "base/memory/ref_counted.h"

namespace base {
class SingleThreadTaskRunner;
class SequencedTaskRunner;
}  // namespace base

namespace mojo {
namespace internal {

// Returns the SequencedTaskRunner to use from the optional user-provided
// SingleThreadTaskRunner. If |runner| is provided non-null, it is returned.
// Otherwise, the current SequencedTaskRunner is returned. If |runner| is
// non-null, it must run on the current thread.
scoped_refptr<base::SequencedTaskRunner>
GetTaskRunnerToUseFromUserProvidedTaskRunner(
    scoped_refptr<base::SingleThreadTaskRunner> runner);

}  // namespace internal
}  // namespace mojo

#endif  // MOJO_PUBLIC_CPP_BINDINGS_LIB_TASK_RUNNER_HELPER_H_

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_NACL_COMMON_NACL_DEBUG_EXCEPTION_HANDLER_WIN_H_
#define COMPONENTS_NACL_COMMON_NACL_DEBUG_EXCEPTION_HANDLER_WIN_H_

#include "base/callback.h"
#include "base/memory/ref_counted.h"
#include "base/process/process.h"
#include "base/single_thread_task_runner.h"

namespace base {
class SingleThreadTaskRunner;
}

void NaClStartDebugExceptionHandlerThread(
    base::Process nacl_process,
    const std::string& startup_info,
    scoped_refptr<base::SingleThreadTaskRunner> task_runner,
    const base::Callback<void(bool)>& on_connected);

#endif  // COMPONENTS_NACL_COMMON_NACL_DEBUG_EXCEPTION_HANDLER_WIN_H_

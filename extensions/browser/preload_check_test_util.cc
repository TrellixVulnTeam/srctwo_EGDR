// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/preload_check_test_util.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/run_loop.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "extensions/common/extension.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace extensions {

// PreloadCheckRunner:
PreloadCheckRunner::PreloadCheckRunner() : called_(false) {}
PreloadCheckRunner::~PreloadCheckRunner() {}

void PreloadCheckRunner::Run(PreloadCheck* check) {
  check->Start(GetCallback());
}

void PreloadCheckRunner::RunUntilComplete(PreloadCheck* check) {
  Run(check);
  ASSERT_FALSE(called_);

  WaitForComplete();
  ASSERT_TRUE(called_);
}

void PreloadCheckRunner::WaitForComplete() {
  run_loop_ = std::make_unique<base::RunLoop>();
  run_loop_->Run();
}

void PreloadCheckRunner::WaitForIdle() {
  run_loop_ = std::make_unique<base::RunLoop>();
  run_loop_->RunUntilIdle();
}

PreloadCheck::ResultCallback PreloadCheckRunner::GetCallback() {
  return base::Bind(&PreloadCheckRunner::OnCheckComplete,
                    base::Unretained(this));
}

void PreloadCheckRunner::OnCheckComplete(PreloadCheck::Errors errors) {
  ASSERT_FALSE(called_);
  called_ = true;
  errors_ = errors;

  if (run_loop_)
    run_loop_->Quit();
}

// PreloadCheckStub:
PreloadCheckStub::PreloadCheckStub(const Errors& errors)
    : PreloadCheck(nullptr), errors_(errors), weak_ptr_factory_(this) {}

PreloadCheckStub::~PreloadCheckStub() {}

void PreloadCheckStub::Start(ResultCallback callback) {
  DCHECK(!callback.is_null());
  started_ = true;
  if (is_async_) {
    // TODO(michaelpg): Bind the callback directly and remove RunCallback
    // once crbug.com/704027 is addressed.
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE,
        base::Bind(&PreloadCheckStub::RunCallback,
                   weak_ptr_factory_.GetWeakPtr(), base::Passed(&callback)));
  } else {
    std::move(callback).Run(errors_);
  }
}

void PreloadCheckStub::RunCallback(ResultCallback callback) {
  std::move(callback).Run(errors_);
}

}  // namespace extensions

// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/password_manager/sync/browser/password_model_worker.h"

#include <utility>

#include "components/password_manager/core/browser/password_store.h"

namespace browser_sync {

PasswordModelWorker::PasswordModelWorker(
    const scoped_refptr<password_manager::PasswordStore>& password_store)
    : password_store_(password_store) {
  DCHECK(password_store.get());
}

syncer::ModelSafeGroup PasswordModelWorker::GetModelSafeGroup() {
  return syncer::GROUP_PASSWORD;
}

bool PasswordModelWorker::IsOnModelSequence() {
  // Ideally PasswordStore would expose a way to check whether this is the
  // thread it does work on. Since it doesn't, just return true to bypass a
  // CHECK in the sync code.
  return true;
}

PasswordModelWorker::~PasswordModelWorker() {}

void PasswordModelWorker::ScheduleWork(base::OnceClosure work) {
  base::AutoLock lock(password_store_lock_);
  if (password_store_) {
    password_store_->ScheduleTask(
        base::Bind([](base::OnceClosure work) { std::move(work).Run(); },
                   base::Passed(std::move(work))));
  }
}

void PasswordModelWorker::RequestStop() {
  ModelSafeWorker::RequestStop();

  base::AutoLock lock(password_store_lock_);
  password_store_ = NULL;
}

}  // namespace browser_sync

// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/password_manager/core/browser/password_store_consumer.h"

#include "components/password_manager/core/browser/statistics_table.h"

namespace password_manager {

PasswordStoreConsumer::PasswordStoreConsumer() : weak_ptr_factory_(this) {
}

PasswordStoreConsumer::~PasswordStoreConsumer() {
}

void PasswordStoreConsumer::OnGetSiteStatistics(
    std::vector<InteractionsStats> stats) {}

void PasswordStoreConsumer::CancelAllRequests() {
  cancelable_task_tracker_.TryCancelAll();
  weak_ptr_factory_.InvalidateWeakPtrs();
}

}  // namespace password_manager

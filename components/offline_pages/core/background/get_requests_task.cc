// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/offline_pages/core/background/get_requests_task.h"

#include <vector>

#include "base/bind.h"

namespace offline_pages {

GetRequestsTask::GetRequestsTask(
    RequestQueueStore* store,
    const RequestQueueStore::GetRequestsCallback& callback)
    : store_(store), callback_(callback), weak_ptr_factory_(this) {}

GetRequestsTask::~GetRequestsTask() {}

void GetRequestsTask::Run() {
  ReadRequest();
}

void GetRequestsTask::ReadRequest() {
  store_->GetRequests(base::Bind(&GetRequestsTask::CompleteWithResult,
                                 weak_ptr_factory_.GetWeakPtr()));
}

void GetRequestsTask::CompleteWithResult(
    bool success,
    std::vector<std::unique_ptr<SavePageRequest>> requests) {
  callback_.Run(success, std::move(requests));
  TaskComplete();
}

}  // namespace offline_pages

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/offline_pages/core/background/change_requests_state_task.h"

#include "base/bind.h"

namespace offline_pages {

ChangeRequestsStateTask::ChangeRequestsStateTask(
    RequestQueueStore* store,
    const std::vector<int64_t>& request_ids,
    const SavePageRequest::RequestState new_state,
    const RequestQueueStore::UpdateCallback& callback)
    : store_(store),
      request_ids_(request_ids.begin(), request_ids.end()),
      new_state_(new_state),
      callback_(callback),
      weak_ptr_factory_(this) {}

ChangeRequestsStateTask::~ChangeRequestsStateTask() {}

void ChangeRequestsStateTask::Run() {
  ReadRequests();
}

void ChangeRequestsStateTask::ReadRequests() {
  std::vector<int64_t> request_ids(request_ids_.begin(), request_ids_.end());
  store_->GetRequestsByIds(request_ids,
                           base::Bind(&ChangeRequestsStateTask::UpdateRequests,
                                      weak_ptr_factory_.GetWeakPtr()));
}

void ChangeRequestsStateTask::UpdateRequests(
    std::unique_ptr<UpdateRequestsResult> read_result) {
  if (read_result->store_state != StoreState::LOADED ||
      read_result->updated_items.empty()) {
    UpdateCompleted(std::move(read_result));
    return;
  }

  // We are only going to make an update to the items that were found. Statuses
  // of the missing items will be added at the end.
  std::vector<SavePageRequest> items_to_update;
  for (auto request : read_result->updated_items) {
    request.set_request_state(new_state_);
    items_to_update.push_back(request);
  }

  store_->UpdateRequests(items_to_update,
                         base::Bind(&ChangeRequestsStateTask::UpdateCompleted,
                                    weak_ptr_factory_.GetWeakPtr()));
}

void ChangeRequestsStateTask::UpdateCompleted(
    std::unique_ptr<UpdateRequestsResult> update_result) {
  // Because the first step might not have found some of the items, we should
  // look their IDs now and include in the final result as not found.

  // Look up the missing items by removing the items present in the result
  // statuses from original list of request IDs.
  for (const auto& id_status_pair : update_result->item_statuses)
    request_ids_.erase(id_status_pair.first);

  // Update the final result for the items that are left in |request_ids_|, as
  // these are identified as being missing from the final result.
  for (int64_t request_id : request_ids_) {
    update_result->item_statuses.push_back(
        std::make_pair(request_id, ItemActionStatus::NOT_FOUND));
  }

  callback_.Run(std::move(update_result));
  TaskComplete();
}

}  // namespace offline_pages

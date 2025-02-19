// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/offline_pages/core/background/pick_request_task.h"

#include <unordered_set>

#include "base/bind.h"
#include "base/logging.h"
#include "base/time/time.h"
#include "components/offline_pages/core/background/device_conditions.h"
#include "components/offline_pages/core/background/offliner_policy.h"
#include "components/offline_pages/core/background/offliner_policy_utils.h"
#include "components/offline_pages/core/background/request_coordinator_event_logger.h"
#include "components/offline_pages/core/background/request_notifier.h"
#include "components/offline_pages/core/background/request_queue_store.h"
#include "components/offline_pages/core/background/save_page_request.h"

namespace {
template <typename T>
int signum(T t) {
  return (T(0) < t) - (t < T(0));
}

bool kCleanupNeeded = true;
bool kNonUserRequestsFound = true;

#define CALL_MEMBER_FUNCTION(object, ptrToMember) ((object)->*(ptrToMember))
}  // namespace

namespace offline_pages {

PickRequestTask::PickRequestTask(
    RequestQueueStore* store,
    OfflinerPolicy* policy,
    RequestPickedCallback picked_callback,
    RequestNotPickedCallback not_picked_callback,
    RequestCountCallback request_count_callback,
    DeviceConditions& device_conditions,
    const std::set<int64_t>& disabled_requests,
    base::circular_deque<int64_t>& prioritized_requests)
    : store_(store),
      policy_(policy),
      picked_callback_(picked_callback),
      not_picked_callback_(not_picked_callback),
      request_count_callback_(request_count_callback),
      disabled_requests_(disabled_requests),
      prioritized_requests_(prioritized_requests),
      weak_ptr_factory_(this) {
  device_conditions_.reset(new DeviceConditions(device_conditions));
}

PickRequestTask::~PickRequestTask() {}

void PickRequestTask::Run() {
  GetRequests();
}

void PickRequestTask::GetRequests() {
  // Get all the requests from the queue, we will classify them in the callback.
  store_->GetRequests(
      base::Bind(&PickRequestTask::Choose, weak_ptr_factory_.GetWeakPtr()));
}

void PickRequestTask::Choose(
    bool success,
    std::vector<std::unique_ptr<SavePageRequest>> requests) {
  // If there is nothing to do, return right away.
  if (requests.empty()) {
    request_count_callback_.Run(requests.size(), 0);
    not_picked_callback_.Run(!kNonUserRequestsFound, !kCleanupNeeded);
    TaskComplete();
    return;
  }

  // Pick the most deserving request for our conditions.
  const SavePageRequest* picked_request = nullptr;

  RequestCompareFunction comparator = nullptr;

  // Choose which comparison function to use based on policy.
  if (policy_->RetryCountIsMoreImportantThanRecency())
    comparator = &PickRequestTask::RetryCountFirstCompareFunction;
  else
    comparator = &PickRequestTask::RecencyFirstCompareFunction;

  bool non_user_requested_tasks_remaining = false;
  bool cleanup_needed = false;

  size_t available_request_count = 0;
  // Request ids which are available for picking.
  std::unordered_set<int64_t> available_request_ids;

  // Iterate through the requests, filter out unavailable requests and get other
  // information (if cleanup is needed and number of non-user-requested
  // requests).
  for (const auto& request : requests) {
    // If the request is expired or has exceeded the retry count, skip it.
    if (OfflinerPolicyUtils::CheckRequestExpirationStatus(request.get(),
                                                          policy_) !=
        OfflinerPolicyUtils::RequestExpirationStatus::VALID) {
      cleanup_needed = true;
      continue;
    }
    // If the request is on the disabled list, skip it.
    auto search = disabled_requests_.find(request->request_id());
    if (search != disabled_requests_.end())
      continue;

    // If there are non-user-requested tasks remaining, we need to make sure
    // that they are scheduled after we run out of user requested tasks. Here we
    // detect if any exist. If we don't find any user-requested tasks, we will
    // inform the "not_picked_callback_" that it needs to schedule a task for
    // non-user-requested items, which have different network and power needs.
    if (!request->user_requested())
      non_user_requested_tasks_remaining = true;
    if (request->request_state() == SavePageRequest::RequestState::AVAILABLE) {
      available_request_count++;
    }
    if (!RequestConditionsSatisfied(request.get()))
      continue;
    available_request_ids.insert(request->request_id());
  }
  // Report the request queue counts.
  request_count_callback_.Run(requests.size(), available_request_count);

  // Search for and pick the prioritized request which is available for picking
  // from |available_request_ids|, the closer to the end means higher priority.
  // Also if a request in |prioritized_requests_| doesn't exist in |requests|
  // we're going to remove it.
  // For every ID in |available_request_ids|, there exists a corresponding
  // request in |requests|, so this won't be an infinite loop: either we pick a
  // request, or there's a request being poped from |prioritized_requests_|.
  while (!picked_request && !prioritized_requests_.empty()) {
    if (available_request_ids.count(prioritized_requests_.back()) > 0) {
      for (const auto& request : requests) {
        if (request->request_id() == prioritized_requests_.back()) {
          picked_request = request.get();
          break;
        }
      }
      DCHECK(picked_request);
    } else {
      prioritized_requests_.pop_back();
    }
  }

  // If no request was found from the priority list, find the best request
  // according to current policies.
  if (!picked_request) {
    for (const auto& request : requests) {
      if ((available_request_ids.count(request->request_id()) > 0) &&
          (IsNewRequestBetter(picked_request, request.get(), comparator))) {
        picked_request = request.get();
      }
    }
  }

  // If we have a best request to try next, get the request coodinator to
  // start it.  Otherwise return that we have no candidates.
  if (picked_request != nullptr) {
    picked_callback_.Run(*picked_request, cleanup_needed);
  } else {
    not_picked_callback_.Run(non_user_requested_tasks_remaining,
                             cleanup_needed);
  }

  TaskComplete();
}

// Filter out requests that don't meet the current conditions.  For instance, if
// this is a predictive request, and we are not on WiFi, it should be ignored
// this round.
bool PickRequestTask::RequestConditionsSatisfied(
    const SavePageRequest* request) {
  // If the user did not request the page directly, make sure we are connected
  // to power and have WiFi and sufficient battery remaining before we take this
  // request.
  if (!device_conditions_->IsPowerConnected() &&
      policy_->PowerRequired(request->user_requested())) {
    return false;
  }

  if (device_conditions_->GetNetConnectionType() !=
          net::NetworkChangeNotifier::ConnectionType::CONNECTION_WIFI &&
      policy_->UnmeteredNetworkRequired(request->user_requested())) {
    return false;
  }

  if (device_conditions_->GetBatteryPercentage() <
      policy_->BatteryPercentageRequired(request->user_requested())) {
    return false;
  }

  // If the request is paused, do not consider it.
  if (request->request_state() == SavePageRequest::RequestState::PAUSED)
    return false;

  return true;
}

// Look at policies to decide which requests to prefer.
bool PickRequestTask::IsNewRequestBetter(const SavePageRequest* oldRequest,
                                         const SavePageRequest* newRequest,
                                         RequestCompareFunction comparator) {
  // If there is no old request, the new one is better.
  if (oldRequest == nullptr)
    return true;

  // User requested pages get priority.
  if (newRequest->user_requested() && !oldRequest->user_requested())
    return true;

  // Otherwise, use the comparison function for the current policy, which
  // returns true if the older request is better.
  return !(CALL_MEMBER_FUNCTION(this, comparator)(oldRequest, newRequest));
}

// Compare the results, checking request count before recency.  Returns true if
// left hand side is better, false otherwise.
bool PickRequestTask::RetryCountFirstCompareFunction(
    const SavePageRequest* left,
    const SavePageRequest* right) {
  // Check the attempt count.
  int result = CompareRetryCount(left, right);

  if (result != 0)
    return (result > 0);

  // If we get here, the attempt counts were the same, so check recency.
  result = CompareCreationTime(left, right);

  return (result > 0);
}

// Compare the results, checking recency before request count. Returns true if
// left hand side is better, false otherwise.
bool PickRequestTask::RecencyFirstCompareFunction(
    const SavePageRequest* left,
    const SavePageRequest* right) {
  // Check the recency.
  int result = CompareCreationTime(left, right);

  if (result != 0)
    return (result > 0);

  // If we get here, the recency was the same, so check the attempt count.
  result = CompareRetryCount(left, right);

  return (result > 0);
}

// Compare left and right side, returning 1 if the left side is better
// (preferred by policy), 0 if the same, and -1 if the right side is better.
int PickRequestTask::CompareRetryCount(const SavePageRequest* left,
                                       const SavePageRequest* right) {
  // Check the attempt count.
  int result = signum(left->completed_attempt_count() -
                      right->completed_attempt_count());

  // Flip the direction of comparison if policy prefers fewer retries.
  if (policy_->ShouldPreferUntriedRequests())
    result *= -1;

  return result;
}

// Compare left and right side, returning 1 if the left side is better
// (preferred by policy), 0 if the same, and -1 if the right side is better.
int PickRequestTask::CompareCreationTime(const SavePageRequest* left,
                                         const SavePageRequest* right) {
  // Check the recency.
  base::TimeDelta difference = left->creation_time() - right->creation_time();
  int result = signum(difference.InMilliseconds());

  // Flip the direction of comparison if policy prefers fewer retries.
  if (policy_->ShouldPreferEarlierRequests())
    result *= -1;

  return result;
}

}  // namespace offline_pages

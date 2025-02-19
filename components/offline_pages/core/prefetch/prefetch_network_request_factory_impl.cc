// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/offline_pages/core/prefetch/prefetch_network_request_factory_impl.h"

#include "base/bind.h"
#include "base/memory/ptr_util.h"
#include "components/offline_pages/core/prefetch/generate_page_bundle_request.h"
#include "components/offline_pages/core/prefetch/get_operation_request.h"

namespace {
constexpr int kMaxBundleSizeBytes = 20 * 1024 * 1024;  // 20 MB

// Max concurrent outstanding requests. If more requests asked to be created,
// the requests are silently not created (considered failed). This is used
// as emergency limit that should rarely be encountered in normal operations.
const int kMaxConcurrentRequests = 10;
}  // namespace

namespace offline_pages {

PrefetchNetworkRequestFactoryImpl::PrefetchNetworkRequestFactoryImpl(
    net::URLRequestContextGetter* request_context,
    version_info::Channel channel,
    const std::string& user_agent)
    : request_context_(request_context),
      channel_(channel),
      user_agent_(user_agent),
      weak_factory_(this) {}

PrefetchNetworkRequestFactoryImpl::~PrefetchNetworkRequestFactoryImpl() =
    default;

bool PrefetchNetworkRequestFactoryImpl::HasOutstandingRequests() const {
  return !(generate_page_bundle_requests_.empty() &&
           get_operation_requests_.empty());
}

void PrefetchNetworkRequestFactoryImpl::MakeGeneratePageBundleRequest(
    const std::vector<std::string>& url_strings,
    const std::string& gcm_registration_id,
    const PrefetchRequestFinishedCallback& callback) {
  if (!AddConcurrentRequest())
    return;
  uint64_t request_id = GetNextRequestId();
  generate_page_bundle_requests_[request_id] =
      base::MakeUnique<GeneratePageBundleRequest>(
          user_agent_, gcm_registration_id, kMaxBundleSizeBytes, url_strings,
          channel_, request_context_.get(),
          base::Bind(
              &PrefetchNetworkRequestFactoryImpl::GeneratePageBundleRequestDone,
              weak_factory_.GetWeakPtr(), callback, request_id));
}

std::unique_ptr<std::set<std::string>>
PrefetchNetworkRequestFactoryImpl::GetAllUrlsRequested() const {
  auto result = base::MakeUnique<std::set<std::string>>();
  for (const auto& request_pair : generate_page_bundle_requests_) {
    for (const auto& url : request_pair.second->requested_urls())
      result->insert(url);
  }
  return result;
}

void PrefetchNetworkRequestFactoryImpl::MakeGetOperationRequest(
    const std::string& operation_name,
    const PrefetchRequestFinishedCallback& callback) {
  if (!AddConcurrentRequest())
    return;
  get_operation_requests_[operation_name] =
      base::MakeUnique<GetOperationRequest>(
          operation_name, channel_, request_context_.get(),
          base::Bind(
              &PrefetchNetworkRequestFactoryImpl::GetOperationRequestDone,
              weak_factory_.GetWeakPtr(), callback));
}

void PrefetchNetworkRequestFactoryImpl::GeneratePageBundleRequestDone(
    const PrefetchRequestFinishedCallback& callback,
    uint64_t request_id,
    PrefetchRequestStatus status,
    const std::string& operation_name,
    const std::vector<RenderPageInfo>& pages) {
  callback.Run(status, operation_name, pages);
  generate_page_bundle_requests_.erase(request_id);
  ReleaseConcurrentRequest();
}

void PrefetchNetworkRequestFactoryImpl::GetOperationRequestDone(
    const PrefetchRequestFinishedCallback& callback,
    PrefetchRequestStatus status,
    const std::string& operation_name,
    const std::vector<RenderPageInfo>& pages) {
  callback.Run(status, operation_name, pages);
  get_operation_requests_.erase(operation_name);
  ReleaseConcurrentRequest();
}

GetOperationRequest*
PrefetchNetworkRequestFactoryImpl::FindGetOperationRequestByName(
    const std::string& operation_name) const {
  auto iter = get_operation_requests_.find(operation_name);
  if (iter == get_operation_requests_.end())
    return nullptr;

  return iter->second.get();
}

bool PrefetchNetworkRequestFactoryImpl::AddConcurrentRequest() {
  if (concurrent_request_count_ >= kMaxConcurrentRequests)
    return false;
  ++concurrent_request_count_;
  return true;
}

std::unique_ptr<std::set<std::string>>
PrefetchNetworkRequestFactoryImpl::GetAllOperationNamesRequested() const {
  auto result = base::MakeUnique<std::set<std::string>>();
  for (const auto& request_pair : get_operation_requests_)
    result->insert(request_pair.first);
  return result;
}

void PrefetchNetworkRequestFactoryImpl::ReleaseConcurrentRequest() {
  DCHECK(concurrent_request_count_ > 0);
  --concurrent_request_count_;
}

// In-memory request id, incremented for each new GeneratePageBundleRequest.
uint64_t PrefetchNetworkRequestFactoryImpl::GetNextRequestId() {
  return ++request_id_;
}

}  // namespace offline_pages

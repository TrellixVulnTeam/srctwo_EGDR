// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/certificate_provider/certificate_requests.h"

#include <memory>
#include <set>
#include <utility>

#include "base/bind.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/time/time.h"
#include "base/timer/timer.h"

namespace chromeos {
namespace certificate_provider {

namespace {
const int kGetCertificatesTimeoutInMinutes = 5;
}  // namespace

// Holds state for a single certificate request.
struct CertificateRequests::CertificateRequestState {
  CertificateRequestState() : timeout(false, false) {}

  ~CertificateRequestState() {}

  // Extensions that are too slow are eventually dropped from a request.
  base::Timer timeout;

  // Extensions that this request is still waiting for.
  std::set<std::string> pending_extensions;

  // For every extension that replied to this request, there is one entry from
  // extension id to the reported certificates.
  std::map<std::string, CertificateInfoList> extension_to_certificates;

  // The callback that must be run with the final list of certificates.
  base::Callback<void(net::ClientCertIdentityList)> callback;
};

CertificateRequests::CertificateRequests() {}

CertificateRequests::~CertificateRequests() {}

int CertificateRequests::AddRequest(
    const std::vector<std::string>& extension_ids,
    const base::Callback<void(net::ClientCertIdentityList)>& callback,
    const base::Callback<void(int)>& timeout_callback) {
  std::unique_ptr<CertificateRequestState> state(new CertificateRequestState);
  state->callback = callback;
  state->pending_extensions.insert(extension_ids.begin(), extension_ids.end());

  const int request_id = next_free_request_id_++;
  state->timeout.Start(
      FROM_HERE, base::TimeDelta::FromMinutes(kGetCertificatesTimeoutInMinutes),
      base::Bind(timeout_callback, request_id));

  const auto insert_result =
      requests_.insert(std::make_pair(request_id, std::move(state)));
  DCHECK(insert_result.second) << "request id already in use.";
  return request_id;
}

bool CertificateRequests::SetCertificates(
    const std::string& extension_id,
    int request_id,
    const CertificateInfoList& certificate_infos,
    bool* completed) {
  *completed = false;
  const auto it = requests_.find(request_id);
  if (it == requests_.end())
    return false;

  CertificateRequestState& state = *it->second;
  if (state.pending_extensions.erase(extension_id) == 0)
    return false;

  state.extension_to_certificates[extension_id] = certificate_infos;
  *completed = state.pending_extensions.empty();
  return true;
}

bool CertificateRequests::RemoveRequest(
    int request_id,
    std::map<std::string, CertificateInfoList>* certificates,
    base::Callback<void(net::ClientCertIdentityList)>* callback) {
  const auto it = requests_.find(request_id);
  if (it == requests_.end())
    return false;

  CertificateRequestState& state = *it->second;
  *certificates = state.extension_to_certificates;
  *callback = state.callback;
  requests_.erase(it);
  DVLOG(2) << "Completed certificate request " << request_id;
  return true;
}

std::vector<int> CertificateRequests::DropExtension(
    const std::string& extension_id) {
  std::vector<int> completed_requests;
  for (const auto& entry : requests_) {
    DVLOG(2) << "Remove extension " << extension_id
             << " from certificate request " << entry.first;

    CertificateRequestState& state = *entry.second.get();
    state.pending_extensions.erase(extension_id);
    if (state.pending_extensions.empty())
      completed_requests.push_back(entry.first);
  }
  return completed_requests;
}

}  // namespace certificate_provider
}  // namespace chromeos

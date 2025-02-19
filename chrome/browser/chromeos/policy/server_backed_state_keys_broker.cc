// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/policy/server_backed_state_keys_broker.h"

#include "base/bind.h"
#include "base/location.h"
#include "base/stl_util.h"
#include "base/threading/thread_task_runner_handle.h"
#include "chromeos/dbus/session_manager_client.h"

namespace policy {

namespace {

// Refresh interval for state keys. There's a quantized time component in
// state key generation, so they rotate over time. The quantum size is pretty
// coarse though (currently 2^23 seconds), so simply polling for a new state
// keys once a day is good enough.
constexpr base::TimeDelta kPollInterval = base::TimeDelta::FromDays(1);

}  // namespace

ServerBackedStateKeysBroker::ServerBackedStateKeysBroker(
    chromeos::SessionManagerClient* session_manager_client)
    : session_manager_client_(session_manager_client),
      requested_(false),
      initial_retrieval_completed_(false),
      weak_factory_(this) {}

ServerBackedStateKeysBroker::~ServerBackedStateKeysBroker() {
}

ServerBackedStateKeysBroker::Subscription
ServerBackedStateKeysBroker::RegisterUpdateCallback(
    const base::Closure& callback) {
  if (!available())
    FetchStateKeys();
  return update_callbacks_.Add(callback);
}

void ServerBackedStateKeysBroker::RequestStateKeys(
    const StateKeysCallback& callback) {
  if (pending()) {
    request_callbacks_.push_back(callback);
    FetchStateKeys();
    return;
  }

  if (!callback.is_null())
    callback.Run(state_keys_);
  return;
}

// static
base::TimeDelta ServerBackedStateKeysBroker::GetPollIntervalForTesting() {
  return kPollInterval;
}

void ServerBackedStateKeysBroker::FetchStateKeys() {
  if (!requested_) {
    requested_ = true;
    session_manager_client_->GetServerBackedStateKeys(
        base::Bind(&ServerBackedStateKeysBroker::StoreStateKeys,
                   weak_factory_.GetWeakPtr()));
  }
}

void ServerBackedStateKeysBroker::StoreStateKeys(
    const std::vector<std::string>& state_keys) {
  bool send_notification = !initial_retrieval_completed_;

  requested_ = false;
  initial_retrieval_completed_ = true;
  if (state_keys.empty()) {
    LOG(WARNING) << "Failed to obtain server-backed state keys.";
  } else if (base::ContainsValue(state_keys, std::string())) {
    LOG(WARNING) << "Bad state keys.";
  } else {
    send_notification |= state_keys_ != state_keys;
    state_keys_ = state_keys;
  }

  if (send_notification)
    update_callbacks_.Notify();

  std::vector<StateKeysCallback> callbacks;
  request_callbacks_.swap(callbacks);
  for (std::vector<StateKeysCallback>::const_iterator callback(
           callbacks.begin());
       callback != callbacks.end();
       ++callback) {
    if (!callback->is_null())
      callback->Run(state_keys_);
  }

  base::ThreadTaskRunnerHandle::Get()->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(&ServerBackedStateKeysBroker::FetchStateKeys,
                     weak_factory_.GetWeakPtr()),
      kPollInterval);
}

}  // namespace policy

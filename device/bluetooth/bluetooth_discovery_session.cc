// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/bluetooth/bluetooth_discovery_session.h"

#include <memory>

#include "base/bind.h"
#include "device/bluetooth/bluetooth_adapter.h"
#include "device/bluetooth/bluetooth_discovery_filter.h"

namespace device {

BluetoothDiscoverySession::BluetoothDiscoverySession(
    scoped_refptr<BluetoothAdapter> adapter,
    std::unique_ptr<BluetoothDiscoveryFilter> discovery_filter)
    : active_(true),
      adapter_(adapter),
      discovery_filter_(discovery_filter.release()),
      weak_ptr_factory_(this) {
  DCHECK(adapter_.get());
}

BluetoothDiscoverySession::~BluetoothDiscoverySession() {
  if (active_) {
    Stop(base::Bind(&base::DoNothing), base::Bind(&base::DoNothing));
    MarkAsInactive();
  }
}

bool BluetoothDiscoverySession::IsActive() const {
  return active_;
}

void BluetoothDiscoverySession::Stop(const base::Closure& success_callback,
                                     const ErrorCallback& error_callback) {
  if (!active_) {
    LOG(WARNING) << "Discovery session not active. Cannot stop.";
    BluetoothAdapter::RecordBluetoothDiscoverySessionStopOutcome(
        UMABluetoothDiscoverySessionOutcome::NOT_ACTIVE);
    error_callback.Run();
    return;
  }
  VLOG(1) << "Stopping device discovery session.";
  base::Closure deactive_discovery_session =
      base::Bind(&BluetoothDiscoverySession::DeactivateDiscoverySession,
                 weak_ptr_factory_.GetWeakPtr());

  // Create a callback that runs
  // BluetoothDiscoverySession::DeactivateDiscoverySession if the session still
  // exists, but always runs success_callback.
  base::Closure discovery_session_removed_callback =
      base::Bind(&BluetoothDiscoverySession::OnDiscoverySessionRemoved,
                 deactive_discovery_session, success_callback);
  adapter_->RemoveDiscoverySession(
      discovery_filter_.get(), discovery_session_removed_callback,
      base::Bind(&BluetoothDiscoverySession::OnDiscoverySessionRemovalFailed,
                 error_callback));
}

// static
void BluetoothDiscoverySession::OnDiscoverySessionRemoved(
    const base::Closure& deactivate_discovery_session,
    const base::Closure& success_callback) {
  BluetoothAdapter::RecordBluetoothDiscoverySessionStopOutcome(
      UMABluetoothDiscoverySessionOutcome::SUCCESS);
  deactivate_discovery_session.Run();
  success_callback.Run();
}

// static
void BluetoothDiscoverySession::OnDiscoverySessionRemovalFailed(
    const base::Closure& error_callback,
    UMABluetoothDiscoverySessionOutcome outcome) {
  BluetoothAdapter::RecordBluetoothDiscoverySessionStopOutcome(outcome);
  error_callback.Run();
}

void BluetoothDiscoverySession::DeactivateDiscoverySession() {
  MarkAsInactive();
  discovery_filter_.reset();
}

void BluetoothDiscoverySession::MarkAsInactive() {
  if (!active_)
    return;
  active_ = false;
  adapter_->DiscoverySessionBecameInactive(this);
}

static void IgnoreDiscoveryOutcome(
    const base::Closure& error_callback,
    UMABluetoothDiscoverySessionOutcome outcome) {
  error_callback.Run();
}

void BluetoothDiscoverySession::SetDiscoveryFilter(
    std::unique_ptr<BluetoothDiscoveryFilter> discovery_filter,
    const base::Closure& callback,
    const ErrorCallback& error_callback) {
  discovery_filter_ = std::move(discovery_filter);
  // BluetoothDiscoverySession::SetDiscoveryFilter is only used from a private
  // extension API, so we don't bother histogramming its failures.
  adapter_->SetDiscoveryFilter(
      adapter_->GetMergedDiscoveryFilter(), callback,
      base::Bind(&IgnoreDiscoveryOutcome, error_callback));
}

const BluetoothDiscoveryFilter* BluetoothDiscoverySession::GetDiscoveryFilter()
    const {
  return discovery_filter_.get();
}

}  // namespace device

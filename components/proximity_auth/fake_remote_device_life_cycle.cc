// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/proximity_auth/fake_remote_device_life_cycle.h"

namespace proximity_auth {

FakeRemoteDeviceLifeCycle::FakeRemoteDeviceLifeCycle(
    const cryptauth::RemoteDevice& remote_device)
    : remote_device_(remote_device),
      started_(false),
      state_(RemoteDeviceLifeCycle::State::STOPPED) {}

FakeRemoteDeviceLifeCycle::~FakeRemoteDeviceLifeCycle() {}

void FakeRemoteDeviceLifeCycle::Start() {
  started_ = true;
}

cryptauth::RemoteDevice FakeRemoteDeviceLifeCycle::GetRemoteDevice() const {
  return remote_device_;
}

cryptauth::Connection* FakeRemoteDeviceLifeCycle::GetConnection() const {
  return connection_;
}

RemoteDeviceLifeCycle::State FakeRemoteDeviceLifeCycle::GetState() const {
  return state_;
}

Messenger* FakeRemoteDeviceLifeCycle::GetMessenger() {
  return messenger_;
}

void FakeRemoteDeviceLifeCycle::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void FakeRemoteDeviceLifeCycle::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

void FakeRemoteDeviceLifeCycle::ChangeState(State new_state) {
  State old_state = state_;
  state_ = new_state;
  for (auto& observer : observers_)
    observer.OnLifeCycleStateChanged(old_state, new_state);
}

}  // namespace proximity_auth

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/geolocation/wifi_data_provider.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/threading/thread_task_runner_handle.h"

namespace device {

WifiDataProvider::WifiDataProvider()
    : client_task_runner_(base::ThreadTaskRunnerHandle::Get()) {
  DCHECK(client_task_runner_);
}

WifiDataProvider::~WifiDataProvider() {}

void WifiDataProvider::AddCallback(WifiDataUpdateCallback* callback) {
  callbacks_.insert(callback);
}

bool WifiDataProvider::RemoveCallback(WifiDataUpdateCallback* callback) {
  return callbacks_.erase(callback) == 1;
}

bool WifiDataProvider::has_callbacks() const {
  return !callbacks_.empty();
}

void WifiDataProvider::RunCallbacks() {
  client_task_runner_->PostTask(
      FROM_HERE, base::Bind(&WifiDataProvider::DoRunCallbacks, this));
}

bool WifiDataProvider::CalledOnClientThread() const {
  return client_task_runner()->BelongsToCurrentThread();
}

void WifiDataProvider::DoRunCallbacks() {
  // It's possible that all the callbacks went away whilst this task was
  // pending. This is fine; the loop will be a no-op.
  CallbackSet::const_iterator iter = callbacks_.begin();
  while (iter != callbacks_.end()) {
    WifiDataUpdateCallback* callback = *iter;
    ++iter;  // Advance iter before running, in case callback unregisters.
    callback->Run();
  }
}

}  // namespace device

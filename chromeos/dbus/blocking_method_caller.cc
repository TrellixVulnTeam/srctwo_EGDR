// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/dbus/blocking_method_caller.h"

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/location.h"
#include "base/task_runner.h"
#include "base/threading/thread_restrictions.h"
#include "dbus/bus.h"
#include "dbus/object_proxy.h"
#include "dbus/scoped_dbus_error.h"

namespace chromeos {

namespace {

// This function is a part of CallMethodAndBlock implementation.
void CallMethodAndBlockInternal(std::unique_ptr<dbus::Response>* response,
                                base::ScopedClosureRunner* signaler,
                                dbus::ObjectProxy* proxy,
                                dbus::MethodCall* method_call,
                                dbus::ScopedDBusError* error_out) {
  *response = proxy->CallMethodAndBlockWithErrorDetails(
      method_call, dbus::ObjectProxy::TIMEOUT_USE_DEFAULT, error_out);
}

}  // namespace

BlockingMethodCaller::BlockingMethodCaller(dbus::Bus* bus,
                                           dbus::ObjectProxy* proxy)
    : bus_(bus),
      proxy_(proxy),
      on_blocking_method_call_(
          base::WaitableEvent::ResetPolicy::AUTOMATIC,
          base::WaitableEvent::InitialState::NOT_SIGNALED) {}

BlockingMethodCaller::~BlockingMethodCaller() {
}

std::unique_ptr<dbus::Response> BlockingMethodCaller::CallMethodAndBlock(
    dbus::MethodCall* method_call) {
  dbus::ScopedDBusError error;
  return CallMethodAndBlockWithError(method_call, &error);
}

std::unique_ptr<dbus::Response>
BlockingMethodCaller::CallMethodAndBlockWithError(
    dbus::MethodCall* method_call,
    dbus::ScopedDBusError* error_out) {
  // on_blocking_method_call_->Signal() will be called when |signaler| is
  // destroyed.
  base::Closure signal_task(
      base::Bind(&base::WaitableEvent::Signal,
                 base::Unretained(&on_blocking_method_call_)));
  base::ScopedClosureRunner* signaler =
      new base::ScopedClosureRunner(signal_task);

  std::unique_ptr<dbus::Response> response;
  bus_->GetDBusTaskRunner()->PostTask(
      FROM_HERE,
      base::Bind(&CallMethodAndBlockInternal, &response, base::Owned(signaler),
                 base::Unretained(proxy_), method_call, error_out));
  // http://crbug.com/125360
  base::ThreadRestrictions::ScopedAllowWait allow_wait;
  on_blocking_method_call_.Wait();
  return response;
}

}  // namespace chromeos

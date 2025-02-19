// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/dbus/system_clock_client.h"

#include <stdint.h>

#include "base/bind.h"
#include "base/macros.h"
#include "base/observer_list.h"
#include "dbus/bus.h"
#include "dbus/message.h"
#include "dbus/object_path.h"
#include "dbus/object_proxy.h"
#include "third_party/cros_system_api/dbus/service_constants.h"

namespace chromeos {

// The SystemClockClient implementation used in production.
class SystemClockClientImpl : public SystemClockClient {
 public:
  SystemClockClientImpl()
      : can_set_time_(false),
        can_set_time_initialized_(false),
        system_clock_proxy_(nullptr),
        weak_ptr_factory_(this) {}

  ~SystemClockClientImpl() override {}

  void AddObserver(Observer* observer) override {
    observers_.AddObserver(observer);
  }

  void RemoveObserver(Observer* observer) override {
    observers_.RemoveObserver(observer);
  }

  bool HasObserver(const Observer* observer) const override {
    return observers_.HasObserver(observer);
  }

  void SetTime(int64_t time_in_seconds) override {
    // Always try to set the time, because |can_set_time_| may be stale.
    dbus::MethodCall method_call(system_clock::kSystemClockInterface,
                                 system_clock::kSystemClockSet);
    dbus::MessageWriter writer(&method_call);
    writer.AppendInt64(time_in_seconds);
    system_clock_proxy_->CallMethod(&method_call,
                                    dbus::ObjectProxy::TIMEOUT_USE_DEFAULT,
                                    dbus::ObjectProxy::EmptyResponseCallback());
  }

  bool CanSetTime() override { return can_set_time_; }

 protected:
  void Init(dbus::Bus* bus) override {
    system_clock_proxy_ = bus->GetObjectProxy(
        system_clock::kSystemClockServiceName,
        dbus::ObjectPath(system_clock::kSystemClockServicePath));
    system_clock_proxy_->ConnectToSignal(
        system_clock::kSystemClockInterface,
        system_clock::kSystemClockUpdated,
        base::Bind(&SystemClockClientImpl::TimeUpdatedReceived,
                   weak_ptr_factory_.GetWeakPtr()),
        base::Bind(&SystemClockClientImpl::TimeUpdatedConnected,
                   weak_ptr_factory_.GetWeakPtr()));
    system_clock_proxy_->WaitForServiceToBeAvailable(
        base::Bind(&SystemClockClientImpl::ServiceInitiallyAvailable,
                   weak_ptr_factory_.GetWeakPtr()));
  }

 private:
  // Called once when the service initially becomes available (or immediately if
  // it's already available).
  void ServiceInitiallyAvailable(bool service_is_available) {
    if (service_is_available)
      GetCanSet();
    else
      LOG(ERROR) << "Failed to wait for D-Bus service availability";
  }

  // Called when a TimeUpdated signal is received.
  void TimeUpdatedReceived(dbus::Signal* signal) {
    VLOG(1) << "TimeUpdated signal received: " << signal->ToString();
    dbus::MessageReader reader(signal);
    for (auto& observer : observers_)
      observer.SystemClockUpdated();

    // Check if the system clock can be changed now.
    GetCanSet();
  }

  // Called when the TimeUpdated signal is initially connected.
  void TimeUpdatedConnected(const std::string& interface_name,
                            const std::string& signal_name,
                            bool success) {
    LOG_IF(ERROR, !success)
        << "Failed to connect to TimeUpdated signal.";
  }

  // Callback for CanSetTime method.
  void OnGetCanSet(dbus::Response* response) {
    if (!response) {
      VLOG(1) << "CanSetTime request failed.";
      return;
    }

    dbus::MessageReader reader(response);
    bool can_set_time;
    if (!reader.PopBool(&can_set_time)) {
      LOG(ERROR) << "CanSetTime response invalid: " << response->ToString();
      return;
    }

    // Nothing to do if the CanSetTime response hasn't changed.
    if (can_set_time_initialized_ && can_set_time_ == can_set_time)
      return;

    can_set_time_initialized_ = true;
    can_set_time_ = can_set_time;

    for (auto& observer : observers_)
      observer.SystemClockCanSetTimeChanged(can_set_time);
  }

  // Check whether the time can be set.
  void GetCanSet() {
    dbus::MethodCall method_call(system_clock::kSystemClockInterface,
                                 system_clock::kSystemClockCanSet);
    dbus::MessageWriter writer(&method_call);
    system_clock_proxy_->CallMethod(
        &method_call, dbus::ObjectProxy::TIMEOUT_USE_DEFAULT,
        base::BindOnce(&SystemClockClientImpl::OnGetCanSet,
                       weak_ptr_factory_.GetWeakPtr()));
  }

  // Whether the time can be set. Value is false until the first
  // CanSetTime response is received.
  bool can_set_time_;
  bool can_set_time_initialized_;
  dbus::ObjectProxy* system_clock_proxy_;
  base::ObserverList<Observer> observers_;

  base::WeakPtrFactory<SystemClockClientImpl> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(SystemClockClientImpl);
};

// static
SystemClockClient* SystemClockClient::Create() {
  return new SystemClockClientImpl();
}

SystemClockClient::SystemClockClient() {}

}  // namespace chromeos

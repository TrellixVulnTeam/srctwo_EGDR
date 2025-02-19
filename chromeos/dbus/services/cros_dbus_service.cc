// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/dbus/services/cros_dbus_service.h"

#include <stddef.h>
#include <utility>

#include "base/bind.h"
#include "base/memory/ptr_util.h"
#include "base/stl_util.h"
#include "base/sys_info.h"
#include "chromeos/dbus/dbus_thread_manager.h"
#include "dbus/bus.h"
#include "dbus/exported_object.h"
#include "dbus/object_path.h"

namespace chromeos {

// The CrosDBusService implementation used in production, and unit tests.
class CrosDBusServiceImpl : public CrosDBusService {
 public:
  CrosDBusServiceImpl(dbus::Bus* bus,
                      const std::string& service_name,
                      const dbus::ObjectPath& object_path,
                      ServiceProviderList service_providers)
      : service_started_(false),
        origin_thread_id_(base::PlatformThread::CurrentId()),
        bus_(bus),
        service_name_(service_name),
        object_path_(object_path),
        service_providers_(std::move(service_providers)) {
    DCHECK(bus);
    DCHECK(!service_name_.empty());
    DCHECK(object_path_.IsValid());
  }

  ~CrosDBusServiceImpl() override = default;

  // Starts the D-Bus service.
  void Start() {
    // Make sure we're running on the origin thread (i.e. the UI thread in
    // production).
    DCHECK(OnOriginThread());
    DCHECK(!service_started_);

    // There are some situations, described in http://crbug.com/234382#c27,
    // where processes on Linux can wind up stuck in an uninterruptible state
    // for tens of seconds. If this happens when Chrome is trying to exit, this
    // unkillable process can wind up clinging to ownership of |service_name_|
    // while the system is trying to restart the browser. This leads to a fatal
    // situation if we don't allow the new browser instance to replace the old
    // as the owner of |service_name_| as seen in http://crbug.com/234382.
    // Hence, REQUIRE_PRIMARY_ALLOW_REPLACEMENT.
    bus_->RequestOwnership(
        service_name_, dbus::Bus::REQUIRE_PRIMARY_ALLOW_REPLACEMENT,
        base::Bind(&CrosDBusServiceImpl::OnOwnership, base::Unretained(this)));

    exported_object_ = bus_->GetExportedObject(object_path_);
    for (size_t i = 0; i < service_providers_.size(); ++i)
      service_providers_[i]->Start(exported_object_);

    service_started_ = true;
  }

 private:
  // Returns true if the current thread is on the origin thread.
  bool OnOriginThread() {
    return base::PlatformThread::CurrentId() == origin_thread_id_;
  }

  // Called when an ownership request is completed.
  void OnOwnership(const std::string& service_name,
                   bool success) {
    LOG_IF(FATAL, !success) << "Failed to own: " << service_name;
  }

  bool service_started_;
  base::PlatformThreadId origin_thread_id_;
  dbus::Bus* bus_;
  std::string service_name_;
  dbus::ObjectPath object_path_;
  scoped_refptr<dbus::ExportedObject> exported_object_;

  // Service providers that form CrosDBusService.
  ServiceProviderList service_providers_;

  DISALLOW_COPY_AND_ASSIGN(CrosDBusServiceImpl);
};

// The stub CrosDBusService implementation used on Linux desktop,
// which does nothing as of now.
class CrosDBusServiceStubImpl : public CrosDBusService {
 public:
  CrosDBusServiceStubImpl() = default;
  ~CrosDBusServiceStubImpl() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(CrosDBusServiceStubImpl);
};

// static
std::unique_ptr<CrosDBusService> CrosDBusService::Create(
    const std::string& service_name,
    const dbus::ObjectPath& object_path,
    ServiceProviderList service_providers) {
  if (DBusThreadManager::Get()->IsUsingFakes())
    return base::MakeUnique<CrosDBusServiceStubImpl>();

  return CreateRealImpl(DBusThreadManager::Get()->GetSystemBus(), service_name,
                        object_path, std::move(service_providers));
}

// static
CrosDBusService::ServiceProviderList CrosDBusService::CreateServiceProviderList(
    std::unique_ptr<ServiceProviderInterface> provider) {
  ServiceProviderList list;
  list.push_back(std::move(provider));
  return list;
}

// static
std::unique_ptr<CrosDBusService> CrosDBusService::CreateRealImpl(
    dbus::Bus* bus,
    const std::string& service_name,
    const dbus::ObjectPath& object_path,
    ServiceProviderList service_providers) {
  auto service = base::MakeUnique<CrosDBusServiceImpl>(
      bus, service_name, object_path, std::move(service_providers));
  service->Start();
  return std::move(service);
}

CrosDBusService::~CrosDBusService() = default;

CrosDBusService::CrosDBusService() = default;

CrosDBusService::ServiceProviderInterface::~ServiceProviderInterface() =
    default;

}  // namespace chromeos

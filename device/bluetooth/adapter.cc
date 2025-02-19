// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <utility>
#include <vector>

#include "base/memory/ptr_util.h"
#include "device/bluetooth/adapter.h"
#include "device/bluetooth/device.h"
#include "device/bluetooth/discovery_session.h"
#include "device/bluetooth/public/interfaces/connect_result_type_converter.h"
#include "mojo/public/cpp/bindings/strong_binding.h"

namespace bluetooth {

Adapter::Adapter(scoped_refptr<device::BluetoothAdapter> adapter)
    : adapter_(std::move(adapter)), client_(nullptr), weak_ptr_factory_(this) {
  adapter_->AddObserver(this);
}

Adapter::~Adapter() {
  adapter_->RemoveObserver(this);
  adapter_ = nullptr;
}

void Adapter::ConnectToDevice(const std::string& address,
                              ConnectToDeviceCallback callback) {
  device::BluetoothDevice* device = adapter_->GetDevice(address);

  if (!device) {
    std::move(callback).Run(mojom::ConnectResult::DEVICE_NO_LONGER_IN_RANGE,
                            nullptr /* device */);
    return;
  }

  auto copyable_callback = base::AdaptCallbackForRepeating(std::move(callback));
  device->CreateGattConnection(
      base::Bind(&Adapter::OnGattConnected, weak_ptr_factory_.GetWeakPtr(),
                 copyable_callback),
      base::Bind(&Adapter::OnConnectError, weak_ptr_factory_.GetWeakPtr(),
                 copyable_callback));
}

void Adapter::GetDevices(GetDevicesCallback callback) {
  std::vector<mojom::DeviceInfoPtr> devices;

  for (const device::BluetoothDevice* device : adapter_->GetDevices()) {
    mojom::DeviceInfoPtr device_info =
        Device::ConstructDeviceInfoStruct(device);
    devices.push_back(std::move(device_info));
  }

  std::move(callback).Run(std::move(devices));
}

void Adapter::GetInfo(GetInfoCallback callback) {
  mojom::AdapterInfoPtr adapter_info = mojom::AdapterInfo::New();
  adapter_info->address = adapter_->GetAddress();
  adapter_info->name = adapter_->GetName();
  adapter_info->initialized = adapter_->IsInitialized();
  adapter_info->present = adapter_->IsPresent();
  adapter_info->powered = adapter_->IsPowered();
  adapter_info->discoverable = adapter_->IsDiscoverable();
  adapter_info->discovering = adapter_->IsDiscovering();
  std::move(callback).Run(std::move(adapter_info));
}

void Adapter::SetClient(mojom::AdapterClientPtr client) {
  client_ = std::move(client);
}

void Adapter::StartDiscoverySession(StartDiscoverySessionCallback callback) {
  auto copyable_callback = base::AdaptCallbackForRepeating(std::move(callback));
  adapter_->StartDiscoverySession(
      base::Bind(&Adapter::OnStartDiscoverySession,
                 weak_ptr_factory_.GetWeakPtr(), copyable_callback),
      base::Bind(&Adapter::OnDiscoverySessionError,
                 weak_ptr_factory_.GetWeakPtr(), copyable_callback));
}

void Adapter::AdapterPresentChanged(device::BluetoothAdapter* adapter,
                                    bool present) {
  if (client_)
    client_->PresentChanged(present);
}

void Adapter::AdapterPoweredChanged(device::BluetoothAdapter* adapter,
                                    bool powered) {
  if (client_)
    client_->PoweredChanged(powered);
}

void Adapter::AdapterDiscoverableChanged(device::BluetoothAdapter* adapter,
                                         bool discoverable) {
  if (client_)
    client_->DiscoverableChanged(discoverable);
}

void Adapter::AdapterDiscoveringChanged(device::BluetoothAdapter* adapter,
                                        bool discovering) {
  if (client_)
    client_->DiscoveringChanged(discovering);
}

void Adapter::DeviceAdded(device::BluetoothAdapter* adapter,
                          device::BluetoothDevice* device) {
  if (client_) {
    auto device_info = Device::ConstructDeviceInfoStruct(device);
    client_->DeviceAdded(std::move(device_info));
  }
}

void Adapter::DeviceChanged(device::BluetoothAdapter* adapter,
                            device::BluetoothDevice* device) {
  if (client_) {
    auto device_info = Device::ConstructDeviceInfoStruct(device);
    client_->DeviceChanged(std::move(device_info));
  }
}

void Adapter::DeviceRemoved(device::BluetoothAdapter* adapter,
                            device::BluetoothDevice* device) {
  if (client_) {
    auto device_info = Device::ConstructDeviceInfoStruct(device);
    client_->DeviceRemoved(std::move(device_info));
  }
}

void Adapter::OnGattConnected(
    ConnectToDeviceCallback callback,
    std::unique_ptr<device::BluetoothGattConnection> connection) {
  mojom::DevicePtr device_ptr;
  Device::Create(adapter_, std::move(connection),
                 mojo::MakeRequest(&device_ptr));
  std::move(callback).Run(mojom::ConnectResult::SUCCESS, std::move(device_ptr));
}

void Adapter::OnConnectError(
    ConnectToDeviceCallback callback,
    device::BluetoothDevice::ConnectErrorCode error_code) {
  std::move(callback).Run(mojo::ConvertTo<mojom::ConnectResult>(error_code),
                          nullptr /* Device */);
}

void Adapter::OnStartDiscoverySession(
    StartDiscoverySessionCallback callback,
    std::unique_ptr<device::BluetoothDiscoverySession> session) {
  mojom::DiscoverySessionPtr session_ptr;
  mojo::MakeStrongBinding(
      std::make_unique<DiscoverySession>(std::move(session)),
      mojo::MakeRequest(&session_ptr));
  std::move(callback).Run(std::move(session_ptr));
}

void Adapter::OnDiscoverySessionError(StartDiscoverySessionCallback callback) {
  std::move(callback).Run(nullptr /* session */);
}

}  // namespace bluetooth

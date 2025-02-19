// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_BLUETOOTH_BLUETOOTH_REMOTE_GATT_SERVICE_H_
#define DEVICE_BLUETOOTH_BLUETOOTH_REMOTE_GATT_SERVICE_H_

#include <string>
#include <vector>

#include "base/macros.h"
#include "device/bluetooth/bluetooth_export.h"
#include "device/bluetooth/bluetooth_gatt_service.h"

namespace device {

class BluetoothDevice;
class BluetoothRemoteGattCharacteristic;
class BluetoothUUID;

// BluetoothRemoteGattService represents a remote GATT service.
//
// Instances of the BluetoothRemoteGATTService class are used to represent GATT
// attribute hierarchies that have been received from a
// remote Bluetooth GATT peripheral. Such BluetoothRemoteGattService instances
// are constructed and owned by a BluetoothDevice.
//
// Note: We use virtual inheritance on the GATT service since it will be
// inherited by platform specific versions of the GATT service classes also. The
// platform specific remote GATT service classes will inherit both this class
// and their GATT service class, hence causing an inheritance diamond.
class DEVICE_BLUETOOTH_EXPORT BluetoothRemoteGattService
    : public virtual BluetoothGattService {
 public:
  ~BluetoothRemoteGattService() override;

  // Returns the BluetoothDevice that this GATT service was received from, which
  // also owns this service.
  virtual BluetoothDevice* GetDevice() const = 0;

  // List of characteristics that belong to this service.
  virtual std::vector<BluetoothRemoteGattCharacteristic*> GetCharacteristics()
      const = 0;

  // List of GATT services that are included by this service.
  virtual std::vector<BluetoothRemoteGattService*> GetIncludedServices()
      const = 0;

  // Returns the GATT characteristic with identifier |identifier| if it belongs
  // to this GATT service.
  virtual BluetoothRemoteGattCharacteristic* GetCharacteristic(
      const std::string& identifier) const = 0;

  std::vector<BluetoothRemoteGattCharacteristic*> GetCharacteristicsByUUID(
      const BluetoothUUID& characteristic_uuid);

 protected:
  BluetoothRemoteGattService();

 private:
  DISALLOW_COPY_AND_ASSIGN(BluetoothRemoteGattService);
};

}  // namespace device

#endif  // DEVICE_BLUETOOTH_BLUETOOTH_REMOTE_GATT_SERVICE_H_

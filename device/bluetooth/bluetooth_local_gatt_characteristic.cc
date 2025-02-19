// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/bluetooth/bluetooth_local_gatt_characteristic.h"

#include "base/logging.h"
#include "build/build_config.h"
#include "device/bluetooth/bluetooth_local_gatt_service.h"

namespace device {

#if !defined(OS_LINUX)
// static
base::WeakPtr<BluetoothLocalGattCharacteristic>
BluetoothLocalGattCharacteristic::Create(const BluetoothUUID& uuid,
                                         Properties properties,
                                         Permissions permissions,
                                         BluetoothLocalGattService* service) {
  NOTIMPLEMENTED();
  return nullptr;
}
#endif

BluetoothLocalGattCharacteristic::BluetoothLocalGattCharacteristic() {}

BluetoothLocalGattCharacteristic::~BluetoothLocalGattCharacteristic() {}

}  // namespace device

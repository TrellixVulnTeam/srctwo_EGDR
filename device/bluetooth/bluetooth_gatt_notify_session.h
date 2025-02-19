// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_BLUETOOTH_BLUETOOTH_GATT_NOTIFY_SESSION_H_
#define DEVICE_BLUETOOTH_BLUETOOTH_GATT_NOTIFY_SESSION_H_

#include <string>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "device/bluetooth/bluetooth_export.h"

namespace device {

class BluetoothRemoteGattCharacteristic;

// A BluetoothGattNotifySession represents an active session for listening
// to value updates from GATT characteristics that support notifications and/or
// indications. Instances are obtained by calling
// BluetoothRemoteGattCharacteristic::StartNotifySession.
class DEVICE_BLUETOOTH_EXPORT BluetoothGattNotifySession {
 public:
  explicit BluetoothGattNotifySession(
      base::WeakPtr<BluetoothRemoteGattCharacteristic> characteristic);

  // Destructor automatically stops this session.
  virtual ~BluetoothGattNotifySession();

  // Returns the identifier of the associated characteristic.
  virtual std::string GetCharacteristicIdentifier() const;

  // Returns the associated characteristic. This function will return nullptr
  // if the associated characteristic is deleted.
  virtual BluetoothRemoteGattCharacteristic* GetCharacteristic() const;

  // Returns true if this session is active. Notify sessions are active from
  // the time of creation, until they have been stopped, either explicitly, or
  // because the remote device disconnects.
  virtual bool IsActive();

  // Stops this session and calls |callback| upon completion. This won't
  // necessarily stop value updates from the characteristic -- since updates
  // are shared among BluetoothGattNotifySession instances -- but it will
  // terminate this session.
  virtual void Stop(const base::Closure& callback);

 private:
  // The associated characteristic.
  base::WeakPtr<BluetoothRemoteGattCharacteristic> characteristic_;
  std::string characteristic_id_;
  bool active_;

  DISALLOW_COPY_AND_ASSIGN(BluetoothGattNotifySession);
};

}  // namespace device

#endif  // DEVICE_BLUETOOTH_BLUETOOTH_GATT_NOTIFY_SESSION_H_

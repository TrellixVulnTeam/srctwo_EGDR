// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/bluetooth/BluetoothCharacteristicProperties.h"

namespace blink {

BluetoothCharacteristicProperties* BluetoothCharacteristicProperties::Create(
    uint32_t properties) {
  return new BluetoothCharacteristicProperties(properties);
}

bool BluetoothCharacteristicProperties::broadcast() const {
  return properties & Property::kBroadcast;
}

bool BluetoothCharacteristicProperties::read() const {
  return properties & Property::kRead;
}

bool BluetoothCharacteristicProperties::writeWithoutResponse() const {
  return properties & Property::kWriteWithoutResponse;
}

bool BluetoothCharacteristicProperties::write() const {
  return properties & Property::kWrite;
}

bool BluetoothCharacteristicProperties::notify() const {
  return properties & Property::kNotify;
}

bool BluetoothCharacteristicProperties::indicate() const {
  return properties & Property::kIndicate;
}

bool BluetoothCharacteristicProperties::authenticatedSignedWrites() const {
  return properties & Property::kAuthenticatedSignedWrites;
}

bool BluetoothCharacteristicProperties::reliableWrite() const {
  return properties & Property::kReliableWrite;
}

bool BluetoothCharacteristicProperties::writableAuxiliaries() const {
  return properties & Property::kWritableAuxiliaries;
}

BluetoothCharacteristicProperties::BluetoothCharacteristicProperties(
    uint32_t device_properties) {
  DCHECK(device_properties != Property::kNone);
  properties = device_properties;
}

}  // namespace blink

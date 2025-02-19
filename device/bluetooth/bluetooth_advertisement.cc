// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/bluetooth/bluetooth_advertisement.h"

namespace device {

BluetoothAdvertisement::Data::Data(AdvertisementType type)
    : type_(type), include_tx_power_(false) {
}

BluetoothAdvertisement::Data::~Data() {
}

BluetoothAdvertisement::Data::Data()
    : type_(ADVERTISEMENT_TYPE_BROADCAST), include_tx_power_(false) {
}

void BluetoothAdvertisement::AddObserver(
    BluetoothAdvertisement::Observer* observer) {
  CHECK(observer);
  observers_.AddObserver(observer);
}

void BluetoothAdvertisement::RemoveObserver(
    BluetoothAdvertisement::Observer* observer) {
  CHECK(observer);
  observers_.RemoveObserver(observer);
}

BluetoothAdvertisement::BluetoothAdvertisement() {
}
BluetoothAdvertisement::~BluetoothAdvertisement() {
}

}  // namespace device

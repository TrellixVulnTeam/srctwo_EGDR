// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/usb/mock_usb_device_handle.h"

#include "device/usb/usb_device.h"

namespace device {

MockUsbDeviceHandle::MockUsbDeviceHandle(UsbDevice* device) : device_(device) {
}

scoped_refptr<UsbDevice> MockUsbDeviceHandle::GetDevice() const {
  return device_;
}

MockUsbDeviceHandle::~MockUsbDeviceHandle() {
}

}  // namespace device

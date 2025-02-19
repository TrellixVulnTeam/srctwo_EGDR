// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/webusb/USBConfiguration.h"

#include "bindings/core/v8/ExceptionState.h"
#include "device/usb/public/interfaces/device.mojom-blink.h"
#include "modules/webusb/USBDevice.h"
#include "modules/webusb/USBInterface.h"

namespace blink {

USBConfiguration* USBConfiguration::Create(const USBDevice* device,
                                           size_t configuration_index) {
  return new USBConfiguration(device, configuration_index);
}

USBConfiguration* USBConfiguration::Create(const USBDevice* device,
                                           size_t configuration_value,
                                           ExceptionState& exception_state) {
  const auto& configurations = device->Info().configurations;
  for (size_t i = 0; i < configurations.size(); ++i) {
    if (configurations[i]->configuration_value == configuration_value)
      return new USBConfiguration(device, i);
  }
  exception_state.ThrowRangeError("Invalid configuration value.");
  return nullptr;
}

USBConfiguration::USBConfiguration(const USBDevice* device,
                                   size_t configuration_index)
    : device_(device), configuration_index_(configuration_index) {
  DCHECK(device_);
  DCHECK_LT(configuration_index_, device_->Info().configurations.size());
}

const USBDevice* USBConfiguration::Device() const {
  return device_;
}

size_t USBConfiguration::Index() const {
  return configuration_index_;
}

const device::mojom::blink::UsbConfigurationInfo& USBConfiguration::Info()
    const {
  return *device_->Info().configurations[configuration_index_];
}

HeapVector<Member<USBInterface>> USBConfiguration::interfaces() const {
  HeapVector<Member<USBInterface>> interfaces;
  for (size_t i = 0; i < Info().interfaces.size(); ++i)
    interfaces.push_back(USBInterface::Create(this, i));
  return interfaces;
}

DEFINE_TRACE(USBConfiguration) {
  visitor->Trace(device_);
}

}  // namespace blink

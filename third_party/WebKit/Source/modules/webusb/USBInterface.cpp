// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/webusb/USBInterface.h"

#include "bindings/core/v8/ExceptionState.h"
#include "device/usb/public/interfaces/device.mojom-blink.h"
#include "modules/webusb/USBAlternateInterface.h"
#include "modules/webusb/USBConfiguration.h"
#include "modules/webusb/USBDevice.h"

namespace blink {

USBInterface* USBInterface::Create(const USBConfiguration* configuration,
                                   size_t interface_index) {
  return new USBInterface(configuration->Device(), configuration->Index(),
                          interface_index);
}

USBInterface* USBInterface::Create(const USBConfiguration* configuration,
                                   size_t interface_number,
                                   ExceptionState& exception_state) {
  const auto& interfaces = configuration->Info().interfaces;
  for (size_t i = 0; i < interfaces.size(); ++i) {
    if (interfaces[i]->interface_number == interface_number)
      return new USBInterface(configuration->Device(), configuration->Index(),
                              i);
  }
  exception_state.ThrowRangeError("Invalid interface index.");
  return nullptr;
}

USBInterface::USBInterface(const USBDevice* device,
                           size_t configuration_index,
                           size_t interface_index)
    : device_(device),
      configuration_index_(configuration_index),
      interface_index_(interface_index) {
  DCHECK_LT(configuration_index_, device_->Info().configurations.size());
  DCHECK_LT(
      interface_index_,
      device_->Info().configurations[configuration_index_]->interfaces.size());
}

const device::mojom::blink::UsbInterfaceInfo& USBInterface::Info() const {
  return *device_->Info()
              .configurations[configuration_index_]
              ->interfaces[interface_index_];
}

USBAlternateInterface* USBInterface::alternate() const {
  if (device_->IsInterfaceClaimed(configuration_index_, interface_index_))
    return USBAlternateInterface::Create(
        this, device_->SelectedAlternateInterface(interface_index_));
  return nullptr;
}

HeapVector<Member<USBAlternateInterface>> USBInterface::alternates() const {
  HeapVector<Member<USBAlternateInterface>> alternates;
  for (size_t i = 0; i < Info().alternates.size(); ++i)
    alternates.push_back(USBAlternateInterface::Create(this, i));
  return alternates;
}

bool USBInterface::claimed() const {
  return device_->IsInterfaceClaimed(configuration_index_, interface_index_);
}

DEFINE_TRACE(USBInterface) {
  visitor->Trace(device_);
}

}  // namespace blink

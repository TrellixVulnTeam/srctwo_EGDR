// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_USB_MOJO_DEVICE_IMPL_H_
#define DEVICE_USB_MOJO_DEVICE_IMPL_H_

#include <stdint.h>
#include <vector>

#include "base/callback_forward.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/scoped_observer.h"
#include "device/usb/mojo/permission_provider.h"
#include "device/usb/public/interfaces/device.mojom.h"
#include "device/usb/usb_device.h"
#include "device/usb/usb_device_handle.h"
#include "mojo/public/cpp/bindings/strong_binding.h"

namespace device {
namespace usb {

class PermissionProvider;

// Implementation of the public Device interface. Instances of this class are
// constructed by DeviceManagerImpl and are strongly bound to their MessagePipe
// lifetime.
class DeviceImpl : public mojom::UsbDevice, public device::UsbDevice::Observer {
 public:
  static void Create(scoped_refptr<device::UsbDevice> device,
                     base::WeakPtr<PermissionProvider> permission_provider,
                     mojom::UsbDeviceRequest request);

  ~DeviceImpl() override;

 private:
  DeviceImpl(scoped_refptr<device::UsbDevice> device,
             base::WeakPtr<PermissionProvider> permission_provider);

  // Closes the device if it's open. This will always set |device_handle_| to
  // null.
  void CloseHandle();

  // Checks interface permissions for control transfers.
  bool HasControlTransferPermission(
      mojom::UsbControlTransferRecipient recipient,
      uint16_t index);

  // Handles completion of an open request.
  static void OnOpen(base::WeakPtr<DeviceImpl> device,
                     OpenCallback callback,
                     scoped_refptr<device::UsbDeviceHandle> handle);
  void OnPermissionGrantedForOpen(OpenCallback callback, bool granted);

  // Device implementation:
  void Open(OpenCallback callback) override;
  void Close(CloseCallback callback) override;
  void SetConfiguration(uint8_t value,
                        SetConfigurationCallback callback) override;
  void ClaimInterface(uint8_t interface_number,
                      ClaimInterfaceCallback callback) override;
  void ReleaseInterface(uint8_t interface_number,
                        ReleaseInterfaceCallback callback) override;
  void SetInterfaceAlternateSetting(
      uint8_t interface_number,
      uint8_t alternate_setting,
      SetInterfaceAlternateSettingCallback callback) override;
  void Reset(ResetCallback callback) override;
  void ClearHalt(uint8_t endpoint, ClearHaltCallback callback) override;
  void ControlTransferIn(mojom::UsbControlTransferParamsPtr params,
                         uint32_t length,
                         uint32_t timeout,
                         ControlTransferInCallback callback) override;
  void ControlTransferOut(mojom::UsbControlTransferParamsPtr params,
                          const std::vector<uint8_t>& data,
                          uint32_t timeout,
                          ControlTransferOutCallback callback) override;
  void GenericTransferIn(uint8_t endpoint_number,
                         uint32_t length,
                         uint32_t timeout,
                         GenericTransferInCallback callback) override;
  void GenericTransferOut(uint8_t endpoint_number,
                          const std::vector<uint8_t>& data,
                          uint32_t timeout,
                          GenericTransferOutCallback callback) override;
  void IsochronousTransferIn(uint8_t endpoint_number,
                             const std::vector<uint32_t>& packet_lengths,
                             uint32_t timeout,
                             IsochronousTransferInCallback callback) override;
  void IsochronousTransferOut(uint8_t endpoint_number,
                              const std::vector<uint8_t>& data,
                              const std::vector<uint32_t>& packet_lengths,
                              uint32_t timeout,
                              IsochronousTransferOutCallback callback) override;

  // device::UsbDevice::Observer implementation:
  void OnDeviceRemoved(scoped_refptr<device::UsbDevice> device) override;

  const scoped_refptr<device::UsbDevice> device_;
  base::WeakPtr<PermissionProvider> permission_provider_;
  ScopedObserver<device::UsbDevice, device::UsbDevice::Observer> observer_;

  // The device handle. Will be null before the device is opened and after it
  // has been closed.
  scoped_refptr<UsbDeviceHandle> device_handle_;

  mojo::StrongBindingPtr<mojom::UsbDevice> binding_;
  base::WeakPtrFactory<DeviceImpl> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(DeviceImpl);
};

}  // namespace usb
}  // namespace device

#endif  // DEVICE_USB_MOJO_DEVICE_IMPL_H_

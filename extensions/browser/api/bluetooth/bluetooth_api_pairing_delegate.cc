// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/api/bluetooth/bluetooth_api_pairing_delegate.h"

#include <memory>
#include <utility>

#include "base/values.h"
#include "content/public/browser/browser_context.h"
#include "device/bluetooth/bluetooth_device.h"
#include "extensions/browser/api/bluetooth/bluetooth_api_utils.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/api/bluetooth_private.h"

namespace extensions {

namespace bt_private = api::bluetooth_private;

namespace {

void PopulatePairingEvent(const device::BluetoothDevice* device,
                          bt_private::PairingEventType type,
                          bt_private::PairingEvent* out) {
  api::bluetooth::BluetoothDeviceToApiDevice(*device, &out->device);
  out->pairing = type;
}

}  // namespace

BluetoothApiPairingDelegate::BluetoothApiPairingDelegate(
    content::BrowserContext* browser_context)
    : browser_context_(browser_context) {}

BluetoothApiPairingDelegate::~BluetoothApiPairingDelegate() {}

void BluetoothApiPairingDelegate::RequestPinCode(
    device::BluetoothDevice* device) {
  bt_private::PairingEvent event;
  PopulatePairingEvent(
      device, bt_private::PAIRING_EVENT_TYPE_REQUESTPINCODE, &event);
  DispatchPairingEvent(event);
}

void BluetoothApiPairingDelegate::RequestPasskey(
    device::BluetoothDevice* device) {
  bt_private::PairingEvent event;
  PopulatePairingEvent(
      device, bt_private::PAIRING_EVENT_TYPE_REQUESTPASSKEY, &event);
  DispatchPairingEvent(event);
}

void BluetoothApiPairingDelegate::DisplayPinCode(
    device::BluetoothDevice* device,
    const std::string& pincode) {
  bt_private::PairingEvent event;
  PopulatePairingEvent(
      device, bt_private::PAIRING_EVENT_TYPE_DISPLAYPINCODE, &event);
  event.pincode.reset(new std::string(pincode));
  DispatchPairingEvent(event);
}

void BluetoothApiPairingDelegate::DisplayPasskey(
    device::BluetoothDevice* device,
    uint32_t passkey) {
  bt_private::PairingEvent event;
  PopulatePairingEvent(
      device, bt_private::PAIRING_EVENT_TYPE_DISPLAYPASSKEY, &event);
  event.passkey.reset(new int(passkey));
  DispatchPairingEvent(event);
}

void BluetoothApiPairingDelegate::KeysEntered(device::BluetoothDevice* device,
                                              uint32_t entered) {
  bt_private::PairingEvent event;
  PopulatePairingEvent(
      device, bt_private::PAIRING_EVENT_TYPE_KEYSENTERED, &event);
  event.entered_key.reset(new int(entered));
  DispatchPairingEvent(event);
}

void BluetoothApiPairingDelegate::ConfirmPasskey(
    device::BluetoothDevice* device,
    uint32_t passkey) {
  bt_private::PairingEvent event;
  PopulatePairingEvent(
      device, bt_private::PAIRING_EVENT_TYPE_CONFIRMPASSKEY, &event);
  event.passkey.reset(new int(passkey));
  DispatchPairingEvent(event);
}

void BluetoothApiPairingDelegate::AuthorizePairing(
    device::BluetoothDevice* device) {
  bt_private::PairingEvent event;
  PopulatePairingEvent(
      device, bt_private::PAIRING_EVENT_TYPE_REQUESTAUTHORIZATION, &event);
  DispatchPairingEvent(event);
}

void BluetoothApiPairingDelegate::DispatchPairingEvent(
    const bt_private::PairingEvent& pairing_event) {
  std::unique_ptr<base::ListValue> args =
      bt_private::OnPairing::Create(pairing_event);
  std::unique_ptr<Event> event(new Event(events::BLUETOOTH_PRIVATE_ON_PAIRING,
                                         bt_private::OnPairing::kEventName,
                                         std::move(args)));
  EventRouter::Get(browser_context_)->BroadcastEvent(std::move(event));
}

}  // namespace extensions

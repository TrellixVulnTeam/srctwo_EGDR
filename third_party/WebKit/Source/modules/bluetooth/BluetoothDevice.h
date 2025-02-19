// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BluetoothDevice_h
#define BluetoothDevice_h

#include <memory>
#include "core/dom/ContextLifecycleObserver.h"
#include "modules/EventTargetModules.h"
#include "modules/bluetooth/BluetoothRemoteGATTServer.h"
#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/Heap.h"
#include "platform/wtf/text/WTFString.h"
#include "public/platform/modules/bluetooth/web_bluetooth.mojom-blink.h"

namespace blink {

class Bluetooth;
class BluetoothAttributeInstanceMap;
class BluetoothRemoteGATTCharacteristic;
class BluetoothRemoteGATTDescriptor;
class BluetoothRemoteGATTServer;
class BluetoothRemoteGATTService;
class ScriptPromiseResolver;

// BluetoothDevice represents a physical bluetooth device in the DOM. See IDL.
//
// Callbacks providing WebBluetoothDevice objects are handled by
// CallbackPromiseAdapter templatized with this class. See this class's
// "Interface required by CallbackPromiseAdapter" section and the
// CallbackPromiseAdapter class comments.
class BluetoothDevice final : public EventTargetWithInlineData,
                              public ContextLifecycleObserver {
  DEFINE_WRAPPERTYPEINFO();
  USING_GARBAGE_COLLECTED_MIXIN(BluetoothDevice);

 public:
  BluetoothDevice(ExecutionContext*,
                  mojom::blink::WebBluetoothDevicePtr,
                  Bluetooth*);

  // Interface required by CallbackPromiseAdapter:
  static BluetoothDevice* Take(ScriptPromiseResolver*,
                               mojom::blink::WebBluetoothDevicePtr,
                               Bluetooth*);

  BluetoothRemoteGATTService* GetOrCreateRemoteGATTService(
      mojom::blink::WebBluetoothRemoteGATTServicePtr,
      bool is_primary,
      const String& device_instance_id);
  bool IsValidService(const String& service_instance_id);

  BluetoothRemoteGATTCharacteristic* GetOrCreateRemoteGATTCharacteristic(
      ExecutionContext*,
      mojom::blink::WebBluetoothRemoteGATTCharacteristicPtr,
      BluetoothRemoteGATTService*);
  bool IsValidCharacteristic(const String& characteristic_instance_id);

  BluetoothRemoteGATTDescriptor* GetOrCreateBluetoothRemoteGATTDescriptor(
      mojom::blink::WebBluetoothRemoteGATTDescriptorPtr,
      BluetoothRemoteGATTCharacteristic*);
  bool IsValidDescriptor(const String& descriptor_instance_id);

  // We should disconnect from the device in all of the following cases:
  // 1. When the object gets GarbageCollected e.g. it went out of scope.
  // dispose() is called in this case.
  // 2. When the parent document gets detached e.g. reloading a page.
  // stop() is called in this case.
  // TODO(ortuno): Users should be able to turn on notifications for
  // events on navigator.bluetooth and still remain connected even if the
  // BluetoothDevice object is garbage collected.

  // Performs necessary cleanup when a device disconnects and fires
  // gattserverdisconnected event.
  void ClearAttributeInstanceMapAndFireEvent();

  // EventTarget methods:
  const AtomicString& InterfaceName() const override;
  ExecutionContext* GetExecutionContext() const override;

  Bluetooth* GetBluetooth() { return bluetooth_; }

  // Interface required by Garbage Collection:
  DECLARE_VIRTUAL_TRACE();

  // IDL exposed interface:
  String id() { return device_->id; }
  String name() { return device_->name; }
  BluetoothRemoteGATTServer* gatt() { return gatt_; }

  DEFINE_ATTRIBUTE_EVENT_LISTENER(gattserverdisconnected);

 protected:
  // EventTarget overrides:
  void AddedEventListener(const AtomicString& eventType,
                          RegisteredEventListener&) override;

 private:
  // Holds all GATT Attributes associated with this BluetoothDevice.
  Member<BluetoothAttributeInstanceMap> attribute_instance_map_;

  mojom::blink::WebBluetoothDevicePtr device_;
  Member<BluetoothRemoteGATTServer> gatt_;
  Member<Bluetooth> bluetooth_;
};

}  // namespace blink

#endif  // BluetoothDevice_h

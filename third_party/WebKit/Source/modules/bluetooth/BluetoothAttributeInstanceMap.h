// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BluetoothAttributeInstanceMap_h
#define BluetoothAttributeInstanceMap_h

#include "modules/bluetooth/BluetoothRemoteGATTCharacteristic.h"
#include "modules/bluetooth/BluetoothRemoteGATTDescriptor.h"
#include "modules/bluetooth/BluetoothRemoteGATTService.h"
#include "platform/heap/Handle.h"
#include "platform/heap/Heap.h"
#include <memory>

namespace blink {

class BluetoothDevice;
class ExecutionContext;

// Map that holds all GATT attributes, i.e. BluetoothRemoteGATTService,
// BluetoothRemoteGATTCharacteristic, BluetoothRemoteGATTDescriptor, for
// the BluetoothDevice passed in when constructing the object.
// Methods in this map are used to create or retrieve these attributes.
class BluetoothAttributeInstanceMap final
    : public GarbageCollected<BluetoothAttributeInstanceMap> {
 public:
  BluetoothAttributeInstanceMap(BluetoothDevice*);

  // Constructs a new BluetoothRemoteGATTService object if there was
  // no service with the same instance id and adds it to the map.
  // Otherwise returns the BluetoothRemoteGATTService object already
  // in the map.
  BluetoothRemoteGATTService* GetOrCreateRemoteGATTService(
      mojom::blink::WebBluetoothRemoteGATTServicePtr,
      bool is_primary,
      const String& device_instance_id);

  // Returns true if a BluetoothRemoteGATTService with |serviceInstanceId|
  // is in the map.
  bool ContainsService(const String& service_instance_id);

  // Constructs a new BluetoothRemoteGATTCharacteristic object if there was no
  // characteristic with the same instance id and adds it to the map.
  // Otherwise returns the BluetoothRemoteGATTCharacteristic object already in
  // the map.
  BluetoothRemoteGATTCharacteristic* GetOrCreateRemoteGATTCharacteristic(
      ExecutionContext*,
      mojom::blink::WebBluetoothRemoteGATTCharacteristicPtr,
      BluetoothRemoteGATTService*);

  // Returns true if a BluetoothRemoteGATTCharacteristic with
  // |characteristicInstanceId| is in the map.
  bool ContainsCharacteristic(const String& characteristic_instance_id);

  // Constructs a new BluetoothRemoteGATTDescriptor object if there was no
  // descriptor with the same instance id and adds it to the map.
  // Otherwise returns the BluetoothRemoteGATTDescriptor object already in
  // the map.
  BluetoothRemoteGATTDescriptor* GetOrCreateBluetoothRemoteGATTDescriptor(
      mojom::blink::WebBluetoothRemoteGATTDescriptorPtr,
      BluetoothRemoteGATTCharacteristic*);

  // Returns true if a BluetoothRemoteGATTDescriptor with
  // |descriptorInstanceId| is in the map.
  bool ContainsDescriptor(const String& descriptor_instance_id);

  // Removes all Attributes from the map.
  // TODO(crbug.com/654950): Remove descriptors when implemented.
  void Clear();

  DECLARE_VIRTUAL_TRACE();

 private:
  // BluetoothDevice that owns this map.
  Member<BluetoothDevice> device_;
  // Map of service instance ids to objects.
  HeapHashMap<String, Member<BluetoothRemoteGATTService>> service_id_to_object_;
  // Map of characteristic instance ids to objects.
  HeapHashMap<String, Member<BluetoothRemoteGATTCharacteristic>>
      characteristic_id_to_object_;
  // Map of descriptor instance ids to objects.
  HeapHashMap<String, Member<BluetoothRemoteGATTDescriptor>>
      descriptor_id_to_object_;
};

}  // namespace blink

#endif  // BluetoothAttributeInstanceMap_h

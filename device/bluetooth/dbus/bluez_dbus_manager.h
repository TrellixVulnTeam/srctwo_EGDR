// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_BLUETOOTH_DBUS_BLUEZ_DBUS_MANAGER_H_
#define DEVICE_BLUETOOTH_DBUS_BLUEZ_DBUS_MANAGER_H_

#include <memory>
#include <string>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "device/bluetooth/bluetooth_export.h"
#include "device/bluetooth/dbus/bluetooth_dbus_client_bundle.h"

namespace dbus {
class Bus;
class Response;
class ErrorResponse;
}  // namespace dbus

namespace bluez {

// Style Note: Clients are sorted by names.
class BluetoothAdapterClient;
class BluetoothAgentManagerClient;
class BluetoothDeviceClient;
class BluetoothGattCharacteristicClient;
class BluetoothGattDescriptorClient;
class BluetoothGattManagerClient;
class BluetoothGattServiceClient;
class BluetoothInputClient;
class BluetoothLEAdvertisingManagerClient;
class BluetoothMediaClient;
class BluetoothMediaTransportClient;
class BluetoothProfileManagerClient;
class BluezDBusManagerSetter;

// BluezDBusManager manages manages D-Bus connections and D-Bus clients, which
// depend on the D-Bus thread to ensure the right order of shutdowns for
// the D-Bus thread, the D-Bus connections, and the D-Bus clients.
//
// CALLBACKS IN D-BUS CLIENTS:
//
// D-Bus clients managed by BluezDBusManagerSetter are guaranteed to be deleted
// after the D-Bus thread so the clients don't need to worry if new
// incoming messages arrive from the D-Bus thread during shutdown of the
// clients. The UI message loop is not running during the shutdown hence
// the UI message loop won't post tasks to D-BUS clients during the
// shutdown. However, to be extra cautious, clients should use
// WeakPtrFactory when creating callbacks that run on UI thread. See
// session_manager_client.cc for examples.
//
class DEVICE_BLUETOOTH_EXPORT BluezDBusManager {
 public:
  // Sets the global instance. Must be called before any calls to Get().
  // We explicitly initialize and shut down the global object, rather than
  // making it a Singleton, to ensure clean startup and shutdown.
  // This will initialize real, stub, or fake DBusClients depending on
  // command-line arguments, whether Object Manager is supported and
  // whether this process runs in a real or test environment.
  static void Initialize(dbus::Bus* bus, bool use_dbus_fakes);

  // Returns a BluezDBusManagerSetter instance that allows tests to
  // replace individual D-Bus clients with their own implementations.
  // Also initializes the main BluezDBusManager for testing if necessary.
  static std::unique_ptr<BluezDBusManagerSetter> GetSetterForTesting();

  // Returns true if BluezDBusManager has been initialized. Call this to
  // avoid initializing + shutting down BluezDBusManager more than once.
  static bool IsInitialized();

  // Destroys the global instance.
  static void Shutdown();

  // Gets the global instance. Initialize() must be called first.
  static BluezDBusManager* Get();

  // Returns various D-Bus bus instances, owned by BluezDBusManager.
  dbus::Bus* GetSystemBus();

  // Returns true once we know whether Object Manager is supported or not.
  // Until this method returns true, no classes should try to use the
  // DBus Clients.
  bool IsObjectManagerSupportKnown() { return object_manager_support_known_; }

  // Calls |callback| once we know whether Object Manager is supported or not.
  void CallWhenObjectManagerSupportIsKnown(base::Closure callback);

  // Returns true if Object Manager is supported.
  bool IsObjectManagerSupported() { return object_manager_supported_; }

  // Returns true if |client| is fake.
  bool IsUsingFakes() { return client_bundle_->IsUsingFakes(); }

  // All returned objects are owned by BluezDBusManager.  Do not use these
  // pointers after BluezDBusManager has been shut down.
  BluetoothAdapterClient* GetBluetoothAdapterClient();
  BluetoothLEAdvertisingManagerClient* GetBluetoothLEAdvertisingManagerClient();
  BluetoothAgentManagerClient* GetBluetoothAgentManagerClient();
  BluetoothDeviceClient* GetBluetoothDeviceClient();
  BluetoothGattCharacteristicClient* GetBluetoothGattCharacteristicClient();
  BluetoothGattDescriptorClient* GetBluetoothGattDescriptorClient();
  BluetoothGattManagerClient* GetBluetoothGattManagerClient();
  BluetoothGattServiceClient* GetBluetoothGattServiceClient();
  BluetoothInputClient* GetBluetoothInputClient();
  BluetoothMediaClient* GetBluetoothMediaClient();
  BluetoothMediaTransportClient* GetBluetoothMediaTransportClient();
  BluetoothProfileManagerClient* GetBluetoothProfileManagerClient();

 private:
  friend class BluezDBusManagerSetter;

  // Creates a new BluezDBusManager using the DBusClients set in
  // |client_bundle|.
  explicit BluezDBusManager(dbus::Bus* bus, bool use_stubs);
  ~BluezDBusManager();

  // Creates a global instance of BluezDBusManager. Cannot be called more than
  // once.
  static void CreateGlobalInstance(dbus::Bus* bus, bool use_stubs);

  void OnObjectManagerSupported(dbus::Response* response);
  void OnObjectManagerNotSupported(dbus::ErrorResponse* response);

  // Initializes all currently stored DBusClients with the system bus and
  // performs additional setup.
  void InitializeClients();

  dbus::Bus* bus_;
  std::unique_ptr<BluetoothDBusClientBundle> client_bundle_;

  base::Closure object_manager_support_known_callback_;

  bool object_manager_support_known_;
  bool object_manager_supported_;

  // Note: This should remain the last member so it'll be destroyed and
  // invalidate its weak pointers before any other members are destroyed.
  base::WeakPtrFactory<BluezDBusManager> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(BluezDBusManager);
};

class DEVICE_BLUETOOTH_EXPORT BluezDBusManagerSetter {
 public:
  ~BluezDBusManagerSetter();

  void SetBluetoothAdapterClient(
      std::unique_ptr<BluetoothAdapterClient> client);
  void SetBluetoothLEAdvertisingManagerClient(
      std::unique_ptr<BluetoothLEAdvertisingManagerClient> client);
  void SetBluetoothAgentManagerClient(
      std::unique_ptr<BluetoothAgentManagerClient> client);
  void SetBluetoothDeviceClient(std::unique_ptr<BluetoothDeviceClient> client);
  void SetBluetoothGattCharacteristicClient(
      std::unique_ptr<BluetoothGattCharacteristicClient> client);
  void SetBluetoothGattDescriptorClient(
      std::unique_ptr<BluetoothGattDescriptorClient> client);
  void SetBluetoothGattManagerClient(
      std::unique_ptr<BluetoothGattManagerClient> client);
  void SetBluetoothGattServiceClient(
      std::unique_ptr<BluetoothGattServiceClient> client);
  void SetBluetoothInputClient(std::unique_ptr<BluetoothInputClient> client);
  void SetBluetoothMediaClient(std::unique_ptr<BluetoothMediaClient> client);
  void SetBluetoothMediaTransportClient(
      std::unique_ptr<BluetoothMediaTransportClient> client);
  void SetBluetoothProfileManagerClient(
      std::unique_ptr<BluetoothProfileManagerClient> client);

 private:
  friend class BluezDBusManager;

  BluezDBusManagerSetter();

  DISALLOW_COPY_AND_ASSIGN(BluezDBusManagerSetter);
};

}  // namespace bluez

#endif  // DEVICE_BLUETOOTH_DBUS_BLUEZ_DBUS_MANAGER_H_

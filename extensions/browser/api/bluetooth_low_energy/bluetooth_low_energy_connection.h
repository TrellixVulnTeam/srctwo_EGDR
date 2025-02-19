// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_API_BLUETOOTH_LOW_ENERGY_BLUETOOTH_LOW_ENERGY_CONNECTION_H_
#define EXTENSIONS_BROWSER_API_BLUETOOTH_LOW_ENERGY_BLUETOOTH_LOW_ENERGY_CONNECTION_H_

#include <memory>

#include "base/macros.h"
#include "device/bluetooth/bluetooth_gatt_connection.h"
#include "extensions/browser/api/api_resource.h"
#include "extensions/browser/api/api_resource_manager.h"

namespace extensions {

// An ApiResource wrapper for a device::BluetoothGattConnection.
class BluetoothLowEnergyConnection : public ApiResource {
 public:
  explicit BluetoothLowEnergyConnection(
      bool persistent,
      const std::string& owner_extension_id,
      std::unique_ptr<device::BluetoothGattConnection> connection);
  ~BluetoothLowEnergyConnection() override;

  // Returns a pointer to the underlying connection object.
  device::BluetoothGattConnection* GetConnection() const;

  // ApiResource override.
  bool IsPersistent() const override;

  // This resource should be managed on the UI thread.
  static const content::BrowserThread::ID kThreadId =
      content::BrowserThread::UI;

 private:
  friend class ApiResourceManager<BluetoothLowEnergyConnection>;
  static const char* service_name() {
    return "BluetoothLowEnergyConnectionManager";
  }

  // True, if this resource should be persistent.
  bool persistent_;

  // The connection is owned by this instance and will automatically disconnect
  // when deleted.
  std::unique_ptr<device::BluetoothGattConnection> connection_;

  DISALLOW_COPY_AND_ASSIGN(BluetoothLowEnergyConnection);
};

}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_API_BLUETOOTH_LOW_ENERGY_BLUETOOTH_LOW_ENERGY_CONNECTION_H_

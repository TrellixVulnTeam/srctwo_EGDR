// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_NETWORK_FAKE_NETWORK_DEVICE_HANDLER_H_
#define CHROMEOS_NETWORK_FAKE_NETWORK_DEVICE_HANDLER_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "chromeos/chromeos_export.h"
#include "chromeos/network/network_device_handler.h"

namespace chromeos {

// This is a fake implementation which does nothing. Use this as a base class
// for concrete fake handlers.
class CHROMEOS_EXPORT FakeNetworkDeviceHandler : public NetworkDeviceHandler {
 public:
  FakeNetworkDeviceHandler();
  ~FakeNetworkDeviceHandler() override;

  // NetworkDeviceHandler overrides
  void GetDeviceProperties(
      const std::string& device_path,
      const network_handler::DictionaryResultCallback& callback,
      const network_handler::ErrorCallback& error_callback) const override;

  void SetDeviceProperty(
      const std::string& device_path,
      const std::string& property_name,
      const base::Value& value,
      const base::Closure& callback,
      const network_handler::ErrorCallback& error_callback) override;

  void RequestRefreshIPConfigs(
      const std::string& device_path,
      const base::Closure& callback,
      const network_handler::ErrorCallback& error_callback) override;

  void ProposeScan(
      const std::string& device_path,
      const base::Closure& callback,
      const network_handler::ErrorCallback& error_callback) override;

  void RegisterCellularNetwork(
      const std::string& device_path,
      const std::string& network_id,
      const base::Closure& callback,
      const network_handler::ErrorCallback& error_callback) override;

  void SetCarrier(
      const std::string& device_path,
      const std::string& carrier,
      const base::Closure& callback,
      const network_handler::ErrorCallback& error_callback) override;

  void RequirePin(
      const std::string& device_path,
      bool require_pin,
      const std::string& pin,
      const base::Closure& callback,
      const network_handler::ErrorCallback& error_callback) override;

  void EnterPin(const std::string& device_path,
                const std::string& pin,
                const base::Closure& callback,
                const network_handler::ErrorCallback& error_callback) override;

  void UnblockPin(
      const std::string& device_path,
      const std::string& puk,
      const std::string& new_pin,
      const base::Closure& callback,
      const network_handler::ErrorCallback& error_callback) override;

  void ChangePin(const std::string& device_path,
                 const std::string& old_pin,
                 const std::string& new_pin,
                 const base::Closure& callback,
                 const network_handler::ErrorCallback& error_callback) override;

  void SetCellularAllowRoaming(bool allow_roaming) override;

  void SetWifiTDLSEnabled(
      const std::string& ip_or_mac_address,
      bool enabled,
      const network_handler::StringResultCallback& callback,
      const network_handler::ErrorCallback& error_callback) override;

  void GetWifiTDLSStatus(
      const std::string& ip_or_mac_address,
      const network_handler::StringResultCallback& callback,
      const network_handler::ErrorCallback& error_callback) override;

  void AddWifiWakeOnPacketConnection(
      const net::IPEndPoint& ip_endpoint,
      const base::Closure& callback,
      const network_handler::ErrorCallback& error_callback) override;

  void RemoveWifiWakeOnPacketConnection(
      const net::IPEndPoint& ip_endpoint,
      const base::Closure& callback,
      const network_handler::ErrorCallback& error_callback) override;

  void RemoveAllWifiWakeOnPacketConnections(
      const base::Closure& callback,
      const network_handler::ErrorCallback& error_callback) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(FakeNetworkDeviceHandler);
};

}  // namespace chromeos

#endif  // CHROMEOS_NETWORK_FAKE_NETWORK_DEVICE_HANDLER_H_

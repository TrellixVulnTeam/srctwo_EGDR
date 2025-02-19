// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_DBUS_SHILL_MANAGER_CLIENT_H_
#define CHROMEOS_DBUS_SHILL_MANAGER_CLIENT_H_

#include <string>

#include "base/macros.h"
#include "chromeos/chromeos_export.h"
#include "chromeos/dbus/dbus_client.h"
#include "chromeos/dbus/dbus_method_call_status.h"
#include "chromeos/dbus/shill_client_helper.h"

namespace dbus {
class ObjectPath;
}

namespace chromeos {

class ShillPropertyChangedObserver;

// ShillManagerClient is used to communicate with the Shill Manager
// service.  All methods should be called from the origin thread which
// initializes the DBusThreadManager instance.
class CHROMEOS_EXPORT ShillManagerClient : public DBusClient {
 public:
  typedef ShillClientHelper::PropertyChangedHandler PropertyChangedHandler;
  typedef ShillClientHelper::DictionaryValueCallback DictionaryValueCallback;
  typedef ShillClientHelper::ErrorCallback ErrorCallback;
  typedef ShillClientHelper::StringCallback StringCallback;
  typedef ShillClientHelper::BooleanCallback BooleanCallback;

  // Interface for setting up devices, services, and technologies for testing.
  // Accessed through GetTestInterface(), only implemented in the Stub Impl.
  class TestInterface {
   public:
    virtual void AddDevice(const std::string& device_path) = 0;
    virtual void RemoveDevice(const std::string& device_path) = 0;
    virtual void ClearDevices() = 0;
    virtual void AddTechnology(const std::string& type, bool enabled) = 0;
    virtual void RemoveTechnology(const std::string& type) = 0;
    virtual void SetTechnologyInitializing(const std::string& type,
                                           bool initializing) = 0;
    virtual void AddGeoNetwork(const std::string& technology,
                               const base::DictionaryValue& network) = 0;

    // Does not create an actual profile in the ProfileClient but update the
    // profiles list and sends a notification to observers. This should only be
    // called by the ProfileStub. In all other cases, use
    // ShillProfileClient::TestInterface::AddProfile.
    virtual void AddProfile(const std::string& profile_path) = 0;

    // Used to reset all properties; does not notify observers.
    virtual void ClearProperties() = 0;

    // Set manager property.
    virtual void SetManagerProperty(const std::string& key,
                                    const base::Value& value) = 0;

    // Modify services in the Manager's list.
    virtual void AddManagerService(const std::string& service_path,
                                   bool notify_observers) = 0;
    virtual void RemoveManagerService(const std::string& service_path) = 0;
    virtual void ClearManagerServices() = 0;

    // Called by ShillServiceClient when a service's State property changes,
    // before notifying observers. Sets the DefaultService property to empty
    // if the state changes to a non-connected state.
    virtual void ServiceStateChanged(const std::string& service_path,
                                     const std::string& state) = 0;

    // Called by ShillServiceClient when a service's State or Visibile
    // property changes. If |notify| is true, notifies observers if a list
    // changed. Services are sorted first by active, inactive, or disabled
    // state, then by type.
    virtual void SortManagerServices(bool notify) = 0;

    // Sets up the default fake environment based on default initial states
    // or states provided by the command line.
    virtual void SetupDefaultEnvironment() = 0;

    // Returns the interactive delay specified on the command line, 0 for none.
    virtual int GetInteractiveDelay() const = 0;

    // Sets the 'best' service to connect to on a ConnectToBestServices call.
    virtual void SetBestServiceToConnect(const std::string& service_path) = 0;

   protected:
    virtual ~TestInterface() {}
  };

  // Properties used to verify the origin device.
  struct VerificationProperties {
    VerificationProperties();
    ~VerificationProperties();

    // A string containing a PEM-encoded X.509 certificate for use in verifying
    // the signed data.
    std::string certificate;

    // A string containing a PEM-encoded RSA public key to be used to compare
    // with the one in signedData
    std::string public_key;

    // A string containing a base64-encoded random binary data for use in
    // verifying the signed data.
    std::string nonce;

    // A string containing the identifying data string signed by the device.
    std::string signed_data;

    // A string containing the serial number of the device.
    std::string device_serial;

    // A string containing the SSID of the device. Only set if the device has
    // already been setup once.
    std::string device_ssid;

    // A string containing the BSSID of the device. Only set if the device has
    // already been setup.
    std::string device_bssid;
  };

  ~ShillManagerClient() override;

  // Factory function, creates a new instance which is owned by the caller.
  // For normal usage, access the singleton via DBusThreadManager::Get().
  static ShillManagerClient* Create();

  // Adds a property changed |observer|.
  virtual void AddPropertyChangedObserver(
      ShillPropertyChangedObserver* observer) = 0;

  // Removes a property changed |observer|.
  virtual void RemovePropertyChangedObserver(
      ShillPropertyChangedObserver* observer) = 0;

  // Calls GetProperties method.
  // |callback| is called after the method call succeeds.
  virtual void GetProperties(const DictionaryValueCallback& callback) = 0;

  // Calls GetNetworksForGeolocation method.
  // |callback| is called after the method call succeeds.
  virtual void GetNetworksForGeolocation(
      const DictionaryValueCallback& callback) = 0;

  // Calls SetProperty method.
  // |callback| is called after the method call succeeds.
  virtual void SetProperty(const std::string& name,
                           const base::Value& value,
                           const base::Closure& callback,
                           const ErrorCallback& error_callback) = 0;

  // Calls RequestScan method.
  // |callback| is called after the method call succeeds.
  virtual void RequestScan(const std::string& type,
                           const base::Closure& callback,
                           const ErrorCallback& error_callback) = 0;

  // Calls EnableTechnology method.
  // |callback| is called after the method call succeeds.
  virtual void EnableTechnology(const std::string& type,
                                const base::Closure& callback,
                                const ErrorCallback& error_callback) = 0;

  // Calls DisableTechnology method.
  // |callback| is called after the method call succeeds.
  virtual void DisableTechnology(const std::string& type,
                                 const base::Closure& callback,
                                 const ErrorCallback& error_callback) = 0;

  // Calls ConfigureService method.
  // |callback| is called after the method call succeeds.
  virtual void ConfigureService(const base::DictionaryValue& properties,
                                const ObjectPathCallback& callback,
                                const ErrorCallback& error_callback) = 0;

  // Calls ConfigureServiceForProfile method.
  // |callback| is called with the created service if the method call succeeds.
  virtual void ConfigureServiceForProfile(
      const dbus::ObjectPath& profile_path,
      const base::DictionaryValue& properties,
      const ObjectPathCallback& callback,
      const ErrorCallback& error_callback) = 0;

  // Calls GetService method.
  // |callback| is called after the method call succeeds.
  virtual void GetService(const base::DictionaryValue& properties,
                          const ObjectPathCallback& callback,
                          const ErrorCallback& error_callback) = 0;

  // Verifies that the given data corresponds to a trusted device, and returns
  // true to the callback if it is.
  virtual void VerifyDestination(const VerificationProperties& properties,
                                 const BooleanCallback& callback,
                                 const ErrorCallback& error_callback) = 0;

  // Verifies that the given data corresponds to a trusted device, and if it is,
  // returns the encrypted credentials for connecting to the network represented
  // by the given |service_path|, encrypted using the |public_key| for the
  // trusted device. If the device is not trusted, returns the empty string.
  virtual void VerifyAndEncryptCredentials(
      const VerificationProperties& properties,
      const std::string& service_path,
      const StringCallback& callback,
      const ErrorCallback& error_callback) = 0;

  // Verifies that the given data corresponds to a trusted device, and returns
  // the |data| encrypted using the |public_key| for the trusted device. If the
  // device is not trusted, returns the empty string.
  virtual void VerifyAndEncryptData(const VerificationProperties& properties,
                                    const std::string& data,
                                    const StringCallback& callback,
                                    const ErrorCallback& error_callback) = 0;

  // For each technology present, connects to the "best" service available.
  // Called once the user is logged in and certificates are loaded.
  virtual void ConnectToBestServices(const base::Closure& callback,
                                     const ErrorCallback& error_callback) = 0;

  // Enable or disable network bandwidth throttling, on all interfaces on the
  // system. If |enabled| is true, |upload_rate_kbits| and |download_rate_kbits|
  // are the desired rates (in kbits/s) to throttle to. If |enabled| is false,
  // throttling is off, and the rates are ignored.
  virtual void SetNetworkThrottlingStatus(
      bool enabled,
      uint32_t upload_rate_kbits,
      uint32_t download_rate_kbits,
      const base::Closure& callback,
      const ErrorCallback& error_callback) = 0;

  // Returns an interface for testing (stub only), or returns NULL.
  virtual TestInterface* GetTestInterface() = 0;

 protected:
  friend class ShillManagerClientTest;

  // Create() should be used instead.
  ShillManagerClient();

 private:
  DISALLOW_COPY_AND_ASSIGN(ShillManagerClient);
};

}  // namespace chromeos

#endif  // CHROMEOS_DBUS_SHILL_MANAGER_CLIENT_H_

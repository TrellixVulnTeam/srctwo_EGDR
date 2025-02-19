// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_API_NETWORKING_PRIVATE_NETWORKING_PRIVATE_API_H_
#define EXTENSIONS_BROWSER_API_NETWORKING_PRIVATE_NETWORKING_PRIVATE_API_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "base/values.h"
#include "extensions/browser/extension_function.h"

namespace extensions {

namespace networking_private {

extern const char kErrorAccessToSharedConfig[];
extern const char kErrorInvalidArguments[];
extern const char kErrorInvalidNetworkGuid[];
extern const char kErrorInvalidNetworkOperation[];
extern const char kErrorNetworkUnavailable[];
extern const char kErrorNotReady[];
extern const char kErrorNotSupported[];
extern const char kErrorPolicyControlled[];
extern const char kErrorSimLocked[];

}  // namespace networking_private

// Implements the chrome.networkingPrivate.getProperties method.
class NetworkingPrivateGetPropertiesFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateGetPropertiesFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.getProperties",
                             NETWORKINGPRIVATE_GETPROPERTIES);

 protected:
  ~NetworkingPrivateGetPropertiesFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  void Success(std::unique_ptr<base::DictionaryValue> result);
  void Failure(const std::string& error_name);

  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateGetPropertiesFunction);
};

// Implements the chrome.networkingPrivate.getManagedProperties method.
class NetworkingPrivateGetManagedPropertiesFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateGetManagedPropertiesFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.getManagedProperties",
                             NETWORKINGPRIVATE_GETMANAGEDPROPERTIES);

 protected:
  ~NetworkingPrivateGetManagedPropertiesFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  void Success(std::unique_ptr<base::DictionaryValue> result);
  void Failure(const std::string& error);

  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateGetManagedPropertiesFunction);
};

// Implements the chrome.networkingPrivate.getState method.
class NetworkingPrivateGetStateFunction : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateGetStateFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.getState",
                             NETWORKINGPRIVATE_GETSTATE);

 protected:
  ~NetworkingPrivateGetStateFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  void Success(std::unique_ptr<base::DictionaryValue> result);
  void Failure(const std::string& error);

  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateGetStateFunction);
};

// Implements the chrome.networkingPrivate.setProperties method.
class NetworkingPrivateSetPropertiesFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateSetPropertiesFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.setProperties",
                             NETWORKINGPRIVATE_SETPROPERTIES);

 protected:
  ~NetworkingPrivateSetPropertiesFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  void Success();
  void Failure(const std::string& error);

  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateSetPropertiesFunction);
};

// Implements the chrome.networkingPrivate.createNetwork method.
class NetworkingPrivateCreateNetworkFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateCreateNetworkFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.createNetwork",
                             NETWORKINGPRIVATE_CREATENETWORK);

 protected:
  ~NetworkingPrivateCreateNetworkFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  void Success(const std::string& guid);
  void Failure(const std::string& error);

  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateCreateNetworkFunction);
};

// Implements the chrome.networkingPrivate.createNetwork method.
class NetworkingPrivateForgetNetworkFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateForgetNetworkFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.forgetNetwork",
                             NETWORKINGPRIVATE_FORGETNETWORK);

 protected:
  ~NetworkingPrivateForgetNetworkFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  void Success();
  void Failure(const std::string& error);

  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateForgetNetworkFunction);
};

// Implements the chrome.networkingPrivate.getNetworks method.
class NetworkingPrivateGetNetworksFunction : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateGetNetworksFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.getNetworks",
                             NETWORKINGPRIVATE_GETNETWORKS);

 protected:
  ~NetworkingPrivateGetNetworksFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  void Success(std::unique_ptr<base::ListValue> network_list);
  void Failure(const std::string& error);

  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateGetNetworksFunction);
};

// Implements the chrome.networkingPrivate.getVisibleNetworks method.
class NetworkingPrivateGetVisibleNetworksFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateGetVisibleNetworksFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.getVisibleNetworks",
                             NETWORKINGPRIVATE_GETVISIBLENETWORKS);

 protected:
  ~NetworkingPrivateGetVisibleNetworksFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  void Success(std::unique_ptr<base::ListValue> network_list);
  void Failure(const std::string& error);

  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateGetVisibleNetworksFunction);
};

// Implements the chrome.networkingPrivate.getEnabledNetworkTypes method.
class NetworkingPrivateGetEnabledNetworkTypesFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateGetEnabledNetworkTypesFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.getEnabledNetworkTypes",
                             NETWORKINGPRIVATE_GETENABLEDNETWORKTYPES);

 protected:
  ~NetworkingPrivateGetEnabledNetworkTypesFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateGetEnabledNetworkTypesFunction);
};

// Implements the chrome.networkingPrivate.getDeviceStates method.
class NetworkingPrivateGetDeviceStatesFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateGetDeviceStatesFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.getDeviceStates",
                             NETWORKINGPRIVATE_GETDEVICESTATES);

 protected:
  ~NetworkingPrivateGetDeviceStatesFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateGetDeviceStatesFunction);
};

// Implements the chrome.networkingPrivate.enableNetworkType method.
class NetworkingPrivateEnableNetworkTypeFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateEnableNetworkTypeFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.enableNetworkType",
                             NETWORKINGPRIVATE_ENABLENETWORKTYPE);

 protected:
  ~NetworkingPrivateEnableNetworkTypeFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateEnableNetworkTypeFunction);
};

// Implements the chrome.networkingPrivate.disableNetworkType method.
class NetworkingPrivateDisableNetworkTypeFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateDisableNetworkTypeFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.disableNetworkType",
                             NETWORKINGPRIVATE_DISABLENETWORKTYPE);

 protected:
  ~NetworkingPrivateDisableNetworkTypeFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateDisableNetworkTypeFunction);
};

// Implements the chrome.networkingPrivate.requestNetworkScan method.
class NetworkingPrivateRequestNetworkScanFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateRequestNetworkScanFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.requestNetworkScan",
                             NETWORKINGPRIVATE_REQUESTNETWORKSCAN);

 protected:
  ~NetworkingPrivateRequestNetworkScanFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateRequestNetworkScanFunction);
};

// Implements the chrome.networkingPrivate.startConnect method.
class NetworkingPrivateStartConnectFunction : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateStartConnectFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.startConnect",
                             NETWORKINGPRIVATE_STARTCONNECT);

 protected:
  ~NetworkingPrivateStartConnectFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  void Success();
  void Failure(const std::string& guid, const std::string& error);

  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateStartConnectFunction);
};

// Implements the chrome.networkingPrivate.startDisconnect method.
class NetworkingPrivateStartDisconnectFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateStartDisconnectFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.startDisconnect",
                             NETWORKINGPRIVATE_STARTDISCONNECT);

 protected:
  ~NetworkingPrivateStartDisconnectFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  void Success();
  void Failure(const std::string& error);

  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateStartDisconnectFunction);
};

// Implements the chrome.networkingPrivate.startActivate method.
class NetworkingPrivateStartActivateFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateStartActivateFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.startActivate",
                             NETWORKINGPRIVATE_STARTACTIVATE);

 protected:
  ~NetworkingPrivateStartActivateFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  void Success();
  void Failure(const std::string& error);

  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateStartActivateFunction);
};

// Implements the chrome.networkingPrivate.verifyDestination method.
class NetworkingPrivateVerifyDestinationFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateVerifyDestinationFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.verifyDestination",
                             NETWORKINGPRIVATE_VERIFYDESTINATION);

 protected:
  ~NetworkingPrivateVerifyDestinationFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

  void Success(bool result);
  void Failure(const std::string& error);

 private:
  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateVerifyDestinationFunction);
};

// Implements the chrome.networkingPrivate.verifyAndEncryptCredentials method.
class NetworkingPrivateVerifyAndEncryptCredentialsFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateVerifyAndEncryptCredentialsFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.verifyAndEncryptCredentials",
                             NETWORKINGPRIVATE_VERIFYANDENCRYPTCREDENTIALS);

 protected:
  ~NetworkingPrivateVerifyAndEncryptCredentialsFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

  void Success(const std::string& result);
  void Failure(const std::string& error);

 private:
  DISALLOW_COPY_AND_ASSIGN(
      NetworkingPrivateVerifyAndEncryptCredentialsFunction);
};

// Implements the chrome.networkingPrivate.verifyAndEncryptData method.
class NetworkingPrivateVerifyAndEncryptDataFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateVerifyAndEncryptDataFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.verifyAndEncryptData",
                             NETWORKINGPRIVATE_VERIFYANDENCRYPTDATA);

 protected:
  ~NetworkingPrivateVerifyAndEncryptDataFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

  void Success(const std::string& result);
  void Failure(const std::string& error);

 private:
  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateVerifyAndEncryptDataFunction);
};

// Implements the chrome.networkingPrivate.setWifiTDLSEnabledState method.
class NetworkingPrivateSetWifiTDLSEnabledStateFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateSetWifiTDLSEnabledStateFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.setWifiTDLSEnabledState",
                             NETWORKINGPRIVATE_SETWIFITDLSENABLEDSTATE);

 protected:
  ~NetworkingPrivateSetWifiTDLSEnabledStateFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

  void Success(const std::string& result);
  void Failure(const std::string& error);

 private:
  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateSetWifiTDLSEnabledStateFunction);
};

// Implements the chrome.networkingPrivate.getWifiTDLSStatus method.
class NetworkingPrivateGetWifiTDLSStatusFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateGetWifiTDLSStatusFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.getWifiTDLSStatus",
                             NETWORKINGPRIVATE_GETWIFITDLSSTATUS);

 protected:
  ~NetworkingPrivateGetWifiTDLSStatusFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

  void Success(const std::string& result);
  void Failure(const std::string& error);

 private:
  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateGetWifiTDLSStatusFunction);
};

class NetworkingPrivateGetCaptivePortalStatusFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateGetCaptivePortalStatusFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.getCaptivePortalStatus",
                             NETWORKINGPRIVATE_GETCAPTIVEPORTALSTATUS);

  // ExtensionFunction:
  ResponseAction Run() override;

 protected:
  ~NetworkingPrivateGetCaptivePortalStatusFunction() override;

 private:
  void Success(const std::string& result);
  void Failure(const std::string& error);

  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateGetCaptivePortalStatusFunction);
};

class NetworkingPrivateUnlockCellularSimFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateUnlockCellularSimFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.unlockCellularSim",
                             NETWORKINGPRIVATE_UNLOCKCELLULARSIM);

  // ExtensionFunction:
  ResponseAction Run() override;

 protected:
  ~NetworkingPrivateUnlockCellularSimFunction() override;

 private:
  void Success();
  void Failure(const std::string& error);

  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateUnlockCellularSimFunction);
};

class NetworkingPrivateSetCellularSimStateFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateSetCellularSimStateFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.setCellularSimState",
                             NETWORKINGPRIVATE_SETCELLULARSIMSTATE);

  // ExtensionFunction:
  ResponseAction Run() override;

 protected:
  ~NetworkingPrivateSetCellularSimStateFunction() override;

 private:
  void Success();
  void Failure(const std::string& error);

  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateSetCellularSimStateFunction);
};

class NetworkingPrivateGetGlobalPolicyFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateGetGlobalPolicyFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.getGlobalPolicy",
                             NETWORKINGPRIVATE_GETGLOBALPOLICY);

 protected:
  ~NetworkingPrivateGetGlobalPolicyFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateGetGlobalPolicyFunction);
};

class NetworkingPrivateGetCertificateListsFunction
    : public UIThreadExtensionFunction {
 public:
  NetworkingPrivateGetCertificateListsFunction() {}
  DECLARE_EXTENSION_FUNCTION("networkingPrivate.getCertificateLists",
                             NETWORKINGPRIVATE_GETCERTIFICATELISTS);

 protected:
  ~NetworkingPrivateGetCertificateListsFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateGetCertificateListsFunction);
};

}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_API_NETWORKING_PRIVATE_NETWORKING_PRIVATE_API_H_

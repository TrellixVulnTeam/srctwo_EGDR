// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_API_DEVICE_PERMISSIONS_MANAGER_H_
#define EXTENSIONS_BROWSER_API_DEVICE_PERMISSIONS_MANAGER_H_

#include <stdint.h>

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/scoped_observer.h"
#include "base/strings/string16.h"
#include "base/threading/thread_checker.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"
#include "components/keyed_service/core/keyed_service.h"
#include "device/hid/hid_service.h"
#include "device/hid/public/interfaces/hid.mojom.h"
#include "device/usb/usb_service.h"

namespace base {
template <typename T>
struct DefaultSingletonTraits;
class Value;
}

namespace content {
class BrowserContext;
}

namespace extensions {

// Stores information about a device saved with access granted.
class DevicePermissionEntry : public base::RefCounted<DevicePermissionEntry> {
 public:
  enum class Type {
    USB,
    HID,
  };

  DevicePermissionEntry(scoped_refptr<device::UsbDevice> device);

  DevicePermissionEntry(const device::mojom::HidDeviceInfo& device);
  DevicePermissionEntry(Type type,
                        uint16_t vendor_id,
                        uint16_t product_id,
                        const base::string16& serial_number,
                        const base::string16& manufacturer_string,
                        const base::string16& product_string,
                        const base::Time& last_used);

  // A persistent device is one that can be recognized when it is reconnected
  // and can therefore be remembered persistently by writing information about
  // it to ExtensionPrefs. Currently this means it has a serial number string.
  bool IsPersistent() const;

  // Convert the device to a serializable value, returns a null pointer if the
  // entry is not persistent.
  std::unique_ptr<base::Value> ToValue() const;

  base::string16 GetPermissionMessageString() const;

  Type type() const { return type_; }
  uint16_t vendor_id() const { return vendor_id_; }
  uint16_t product_id() const { return product_id_; }
  const base::string16& serial_number() const { return serial_number_; }
  const base::Time& last_used() const { return last_used_; }

  base::string16 GetManufacturer() const;
  base::string16 GetProduct() const;

 private:
  friend class base::RefCounted<DevicePermissionEntry>;
  friend class DevicePermissionsManager;

  ~DevicePermissionEntry();

  void set_last_used(const base::Time& last_used) { last_used_ = last_used; }

  // The USB device tracked by this entry. Will be nullptr if this entry was
  // restored from ExtensionPrefs or type_ is not Type::USB.
  scoped_refptr<device::UsbDevice> usb_device_;

  // The device guid of hid device tracked by this entry.
  std::string hid_device_guid_;
  // The type of device this entry represents.
  Type type_;
  // The vendor ID of this device.
  uint16_t vendor_id_;
  // The product ID of this device.
  uint16_t product_id_;
  // The serial number (possibly alphanumeric) of this device.
  base::string16 serial_number_;
  // The manufacturer string read from the device (optional).
  base::string16 manufacturer_string_;
  // The product string read from the device (optional).
  base::string16 product_string_;
  // The last time this device was used by the extension.
  base::Time last_used_;
};

// Stores device permissions associated with a particular extension.
class DevicePermissions {
 public:
  virtual ~DevicePermissions();

  // Attempts to find a permission entry matching the given device.
  scoped_refptr<DevicePermissionEntry> FindUsbDeviceEntry(
      scoped_refptr<device::UsbDevice> device) const;
  scoped_refptr<DevicePermissionEntry> FindHidDeviceEntry(
      const device::mojom::HidDeviceInfo& device) const;

  const std::set<scoped_refptr<DevicePermissionEntry>>& entries() const {
    return entries_;
  }

 private:
  friend class DevicePermissionsManager;

  // Reads permissions out of ExtensionPrefs.
  DevicePermissions(content::BrowserContext* context,
                    const std::string& extension_id);

  std::set<scoped_refptr<DevicePermissionEntry>> entries_;
  std::map<device::UsbDevice*, scoped_refptr<DevicePermissionEntry>>
      ephemeral_usb_devices_;
  std::map<std::string, scoped_refptr<DevicePermissionEntry>>
      ephemeral_hid_devices_;

  DISALLOW_COPY_AND_ASSIGN(DevicePermissions);
};

// Manages saved device permissions for all extensions.
class DevicePermissionsManager : public KeyedService,
                                 public device::UsbService::Observer {
 public:
  static DevicePermissionsManager* Get(content::BrowserContext* context);

  static base::string16 GetPermissionMessage(
      uint16_t vendor_id,
      uint16_t product_id,
      const base::string16& manufacturer_string,
      const base::string16& product_string,
      const base::string16& serial_number,
      bool always_include_manufacturer);

  // The DevicePermissions object for a given extension.
  DevicePermissions* GetForExtension(const std::string& extension_id);

  // Equivalent to calling GetForExtension and extracting the permission string
  // for each entry.
  std::vector<base::string16> GetPermissionMessageStrings(
      const std::string& extension_id) const;

  void AllowUsbDevice(const std::string& extension_id,
                      scoped_refptr<device::UsbDevice> device);
  void AllowHidDevice(const std::string& extension_id,
                      const device::mojom::HidDeviceInfo& device);

  // Updates the "last used" timestamp on the given device entry and writes it
  // out to ExtensionPrefs.
  void UpdateLastUsed(const std::string& extension_id,
                      scoped_refptr<DevicePermissionEntry> entry);

  // Revokes permission for the extension to access the given device.
  void RemoveEntry(const std::string& extension_id,
                   scoped_refptr<DevicePermissionEntry> entry);

  // Revokes permission for an ephemeral hid device by its guid.

  void RemoveEntryByHidDeviceGUID(const std::string& guid);

  // Revokes permission for the extension to access all allowed devices.
  void Clear(const std::string& extension_id);

 private:

  friend class DevicePermissionsManagerFactory;
  FRIEND_TEST_ALL_PREFIXES(DevicePermissionsManagerTest, SuspendExtension);

  DevicePermissionsManager(content::BrowserContext* context);
  ~DevicePermissionsManager() override;

  DevicePermissions* GetInternal(const std::string& extension_id) const;

  // UsbService::Observer implementation
  void OnDeviceRemovedCleanup(scoped_refptr<device::UsbDevice> device) override;

  base::ThreadChecker thread_checker_;
  content::BrowserContext* context_;
  std::map<std::string, DevicePermissions*> extension_id_to_device_permissions_;
  ScopedObserver<device::UsbService, device::UsbService::Observer>
      usb_service_observer_;

  DISALLOW_COPY_AND_ASSIGN(DevicePermissionsManager);
};

class DevicePermissionsManagerFactory
    : public BrowserContextKeyedServiceFactory {
 public:
  static DevicePermissionsManager* GetForBrowserContext(
      content::BrowserContext* context);
  static DevicePermissionsManagerFactory* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<DevicePermissionsManagerFactory>;

  DevicePermissionsManagerFactory();
  ~DevicePermissionsManagerFactory() override;

  // BrowserContextKeyedServiceFactory implementation
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;

  DISALLOW_COPY_AND_ASSIGN(DevicePermissionsManagerFactory);
};

}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_API_DEVICE_PERMISSIONS_MANAGER_H_

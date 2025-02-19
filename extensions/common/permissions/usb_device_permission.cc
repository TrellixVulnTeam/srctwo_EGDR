// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/common/permissions/usb_device_permission.h"

#include <string>
#include <utility>
#include <vector>

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/strings/string16.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "device/usb/usb_descriptors.h"
#include "device/usb/usb_device.h"
#include "device/usb/usb_ids.h"
#include "extensions/common/extension.h"
#include "extensions/common/features/behavior_feature.h"
#include "extensions/common/features/feature.h"
#include "extensions/common/features/feature_provider.h"
#include "extensions/common/permissions/permissions_info.h"
#include "extensions/strings/grit/extensions_strings.h"
#include "ui/base/l10n/l10n_util.h"

namespace extensions {

namespace {

const int kHidInterfaceClass = 3;

bool IsInterfaceClassPermissionAlowed(const Extension* extension) {
  const Feature* feature = FeatureProvider::GetBehaviorFeature(
      behavior_feature::kAllowUsbDevicesPermissionInterfaceClass);
  if (!feature)
    return false;
  if (!extension)
    return false;
  return feature->IsAvailableToExtension(extension).is_available();
}

}  // namespace

// static
std::unique_ptr<UsbDevicePermission::CheckParam>
UsbDevicePermission::CheckParam::ForUsbDevice(const Extension* extension,
                                              const device::UsbDevice* device) {
  return CheckParam::ForUsbDeviceAndInterface(
      extension, device, UsbDevicePermissionData::SPECIAL_VALUE_UNSPECIFIED);
}

// static
std::unique_ptr<UsbDevicePermission::CheckParam>
UsbDevicePermission::CheckParam::ForDeviceWithAnyInterfaceClass(
    const Extension* extension,
    uint16_t vendor_id,
    uint16_t product_id,
    int interface_id) {
  return std::make_unique<CheckParam>(extension, vendor_id, product_id,
                                      std::unique_ptr<std::set<int>>(),
                                      interface_id);
}

// static
std::unique_ptr<UsbDevicePermission::CheckParam>
UsbDevicePermission::CheckParam::ForUsbDeviceAndInterface(
    const Extension* extension,
    const device::UsbDevice* device,
    int interface_id) {
  std::unique_ptr<std::set<int>> interface_classes(new std::set<int>());
  // If device class is set, match interface class against it as well. This is
  // to enable filtering devices by device-only class (for example, hubs), which
  // might or might not have an interface with class set to device class value.
  if (device->device_class())
    interface_classes->insert(device->device_class());

  for (const auto& configuration : device->configurations()) {
    for (const auto& interface : configuration.interfaces)
      interface_classes->insert(interface.interface_class);
  }

  return std::make_unique<CheckParam>(
      extension, device->vendor_id(), device->product_id(),
      std::move(interface_classes), interface_id);
}

// static
std::unique_ptr<UsbDevicePermission::CheckParam>
UsbDevicePermission::CheckParam::ForHidDevice(const Extension* extension,
                                              uint16_t vendor_id,
                                              uint16_t product_id) {
  std::unique_ptr<std::set<int>> interface_classes(new std::set<int>());
  interface_classes->insert(kHidInterfaceClass);
  return std::make_unique<UsbDevicePermission::CheckParam>(
      extension, vendor_id, product_id, std::move(interface_classes),
      UsbDevicePermissionData::SPECIAL_VALUE_UNSPECIFIED);
}

UsbDevicePermission::CheckParam::CheckParam(
    const Extension* extension,
    uint16_t vendor_id,
    uint16_t product_id,
    std::unique_ptr<std::set<int>> interface_classes,
    int interface_id)
    : vendor_id(vendor_id),
      product_id(product_id),
      interface_classes(std::move(interface_classes)),
      interface_id(interface_id),
      interface_class_allowed(IsInterfaceClassPermissionAlowed(extension)) {}

UsbDevicePermission::CheckParam::~CheckParam() {}

UsbDevicePermission::UsbDevicePermission(const APIPermissionInfo* info)
    : SetDisjunctionPermission<UsbDevicePermissionData, UsbDevicePermission>(
          info) {}

UsbDevicePermission::~UsbDevicePermission() {}

bool UsbDevicePermission::FromValue(
    const base::Value* value,
    std::string* error,
    std::vector<std::string>* unhandled_permissions) {
  bool parsed_ok =
      SetDisjunctionPermission<UsbDevicePermissionData, UsbDevicePermission>::
          FromValue(value, error, unhandled_permissions);
  if (parsed_ok && data_set_.empty()) {
    if (error)
      *error = "NULL or empty permission list";
    return false;
  }
  return parsed_ok;
}

PermissionIDSet UsbDevicePermission::GetPermissions() const {
  PermissionIDSet ids;

  std::set<uint16_t> unknown_product_vendors;
  bool found_unknown_vendor = false;

  for (const UsbDevicePermissionData& data : data_set_) {
    // Interface class permissions should be only available in kiosk sessions,
    // don't include those in installation warning for now.
    if (data.interface_class() != UsbDevicePermissionData::SPECIAL_VALUE_ANY)
      continue;
    const char* vendor = device::UsbIds::GetVendorName(data.vendor_id());
    if (vendor) {
      const char* product =
          device::UsbIds::GetProductName(data.vendor_id(), data.product_id());
      if (product) {
        base::string16 product_name_and_vendor = l10n_util::GetStringFUTF16(
            IDS_EXTENSION_USB_DEVICE_PRODUCT_NAME_AND_VENDOR,
            base::UTF8ToUTF16(product), base::UTF8ToUTF16(vendor));
        ids.insert(APIPermission::kUsbDevice, product_name_and_vendor);
      } else {
        unknown_product_vendors.insert(data.vendor_id());
      }
    } else {
      found_unknown_vendor = true;
    }
  }

  for (uint16_t vendor_id : unknown_product_vendors) {
    const char* vendor = device::UsbIds::GetVendorName(vendor_id);
    DCHECK(vendor);
    ids.insert(APIPermission::kUsbDeviceUnknownProduct,
               base::UTF8ToUTF16(vendor));
  }

  if (found_unknown_vendor)
    ids.insert(APIPermission::kUsbDeviceUnknownVendor);

  return ids;
}

}  // namespace extensions

// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_BLUETOOTH_BLUETOOTH_ALLOWED_DEVICES_MAP_H_
#define CONTENT_BROWSER_BLUETOOTH_BLUETOOTH_ALLOWED_DEVICES_MAP_H_

#include <map>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "content/common/content_export.h"
#include "url/origin.h"

namespace content {

class BluetoothAllowedDevices;

// Class for keeping track of which origins are allowed to access which
// Bluetooth devices and their services.
class CONTENT_EXPORT BluetoothAllowedDevicesMap
    : public base::RefCountedThreadSafe<BluetoothAllowedDevicesMap> {
 public:
  BluetoothAllowedDevicesMap();

  // Gets a BluetoothAllowedDevices for each origin; creates one if it doesn't
  // exist.
  content::BluetoothAllowedDevices& GetOrCreateAllowedDevices(
      const url::Origin& origin);

  // Clears the data in |origin_to_allowed_devices_map_|.
  void Clear();

 private:
  friend class base::RefCountedThreadSafe<BluetoothAllowedDevicesMap>;
  ~BluetoothAllowedDevicesMap();
  std::map<url::Origin, content::BluetoothAllowedDevices>
      origin_to_allowed_devices_map_;

  DISALLOW_COPY_AND_ASSIGN(BluetoothAllowedDevicesMap);
};

}  //  namespace content

#endif  // CONTENT_BROWSER_BLUETOOTH_BLUETOOTH_ALLOWED_DEVICES_MAP_H_
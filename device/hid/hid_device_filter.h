// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_HID_HID_DEVICE_FILTER_H_
#define DEVICE_HID_HID_DEVICE_FILTER_H_

#include <stdint.h>
#include <vector>

#include "base/memory/ref_counted.h"
#include "device/hid/public/interfaces/hid.mojom.h"

namespace device {

class HidDeviceFilter {
 public:
  HidDeviceFilter();
  ~HidDeviceFilter();

  void SetVendorId(uint16_t vendor_id);
  void SetProductId(uint16_t product_id);
  void SetUsagePage(uint16_t usage_page);
  void SetUsage(uint16_t usage);

  bool Matches(const device::mojom::HidDeviceInfo& device_info) const;

  static bool MatchesAny(const device::mojom::HidDeviceInfo& device_info,
                         const std::vector<HidDeviceFilter>& filters);

 private:
  uint16_t vendor_id_;
  uint16_t product_id_;
  uint16_t usage_page_;
  uint16_t usage_;
  bool vendor_id_set_ : 1;
  bool product_id_set_ : 1;
  bool usage_page_set_ : 1;
  bool usage_set_ : 1;
};

}  // namespace device

#endif  // DEVICE_HID_HID_DEVICE_FILTER_H_

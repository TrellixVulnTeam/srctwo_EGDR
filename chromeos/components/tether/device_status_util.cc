// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/components/tether/device_status_util.h"

namespace chromeos {

namespace tether {

namespace {

const char kDefaultCellCarrierName[] = "unknown-carrier";

// Android signal strength is measured between 0 and 4 (inclusive), but Chrome
// OS signal strength is measured between 0 and 100 (inclusive). In order to
// convert between Android signal strength to Chrome OS signal strength, the
// value must be multiplied by the below value.
const int32_t kAndroidTetherHostToChromeOSSignalStrengthMultiplier = 25;

int32_t ForceBetweenZeroAndOneHundred(int32_t value) {
  return std::min(std::max(value, 0), 100);
}

}  // namespace

void NormalizeDeviceStatus(const DeviceStatus& status,
                           std::string* carrier_out,
                           int32_t* battery_percentage_out,
                           int32_t* signal_strength_out) {
  // Use a sentinel value if carrier information is not available. This value is
  // special-cased and replaced with a localized string in the settings UI.
  if (carrier_out) {
    *carrier_out =
        (!status.has_cell_provider() || status.cell_provider().empty())
            ? kDefaultCellCarrierName
            : status.cell_provider();
  }

  // If battery or signal strength are missing, assume they are 100. For
  // battery percentage, force the value to be between 0 and 100. For signal
  // strength, convert from Android signal strength to Chrome OS signal
  // strength and force the value to be between 0 and 100.
  if (battery_percentage_out) {
    *battery_percentage_out =
        status.has_battery_percentage()
            ? ForceBetweenZeroAndOneHundred(status.battery_percentage())
            : 100;
  }
  if (signal_strength_out) {
    *signal_strength_out =
        status.has_connection_strength()
            ? ForceBetweenZeroAndOneHundred(
                  kAndroidTetherHostToChromeOSSignalStrengthMultiplier *
                  status.connection_strength())
            : 100;
  }
}

}  // namespace tether

}  // namespace chromeos

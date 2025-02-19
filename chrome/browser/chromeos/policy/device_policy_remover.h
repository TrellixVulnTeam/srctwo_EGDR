// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_POLICY_DEVICE_POLICY_REMOVER_H_
#define CHROME_BROWSER_CHROMEOS_POLICY_DEVICE_POLICY_REMOVER_H_

#include <vector>

#include "chrome/browser/chromeos/policy/proto/chrome_device_policy.pb.h"

namespace policy {

// Remove |policies_to_remove| fields from |policies|.
// Vector |policies_to_remove| contains device policy names from
// ChromeDeviceSettingsProto.
// Implementation of this method is generated
// automatically by generate_device_policy_remover.py.
void RemovePolicies(enterprise_management::ChromeDeviceSettingsProto* policies,
                    const std::vector<std::string>& policies_to_remove);

}  // namespace policy

#endif  // CHROME_BROWSER_CHROMEOS_POLICY_DEVICE_POLICY_REMOVER_H_

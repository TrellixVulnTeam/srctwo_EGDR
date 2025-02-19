// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_DEVICE_VIBRATION_VIBRATION_MANAGER_IMPL_H_
#define SERVICES_DEVICE_VIBRATION_VIBRATION_MANAGER_IMPL_H_

#include "services/device/public/interfaces/vibration_manager.mojom.h"

namespace device {

class VibrationManagerImpl {
 public:
  static void Create(mojo::InterfaceRequest<mojom::VibrationManager> request);

  static int64_t milli_seconds_for_testing_;
  static bool cancelled_for_testing_;
};

}  // namespace device

#endif  // SERVICES_DEVICE_VIBRATION_VIBRATION_MANAGER_IMPL_H_

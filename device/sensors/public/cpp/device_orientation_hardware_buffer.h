// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_SENSORS_PUBLIC_CPP_DEVICE_ORIENTATION_HARDWARE_BUFFER_H_
#define DEVICE_SENSORS_PUBLIC_CPP_DEVICE_ORIENTATION_HARDWARE_BUFFER_H_

#include "device/base/synchronization/shared_memory_seqlock_buffer.h"
#include "device/sensors/public/cpp/orientation_data.h"

namespace device {

typedef SharedMemorySeqLockBuffer<OrientationData>
    DeviceOrientationHardwareBuffer;

}  // namespace device

#endif  // DEVICE_SENSORS_PUBLIC_CPP_DEVICE_ORIENTATION_HARDWARE_BUFFER_H_

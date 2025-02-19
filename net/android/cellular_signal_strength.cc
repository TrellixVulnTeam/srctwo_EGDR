// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/android/cellular_signal_strength.h"

#include "jni/AndroidCellularSignalStrength_jni.h"

namespace net {

namespace android {

namespace cellular_signal_strength {

namespace {

// GENERATED_JAVA_ENUM_PACKAGE: org.chromium.net
enum CellularSignalStrengthError {
  // Value returned by CellularSignalStrength APIs when a valid value is
  // unavailable. This value is same as INT32_MIN, but the following code uses
  // the explicit value of INT32_MIN so that the auto-generated Java enums work
  // correctly.
  ERROR_NOT_SUPPORTED = -2147483648,
};

static_assert(
    INT32_MIN == ERROR_NOT_SUPPORTED,
    "CellularSignalStrengthError.ERROR_NOT_SUPPORTED has unexpected value");

}  // namespace

base::Optional<int32_t> GetSignalStrengthLevel() {
  int32_t signal_strength_level =
      Java_AndroidCellularSignalStrength_getSignalStrengthLevel(
          base::android::AttachCurrentThread());
  if (signal_strength_level == ERROR_NOT_SUPPORTED)
    return base::Optional<int32_t>();

  DCHECK_LE(0, signal_strength_level);
  DCHECK_GE(4, signal_strength_level);

  return signal_strength_level;
}

}  // namespace cellular_signal_strength

}  // namespace android

}  // namespace net

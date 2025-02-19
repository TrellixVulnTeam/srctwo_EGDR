// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/bluetooth/android/wrappers.h"

#include "base/android/jni_android.h"
#include "jni/Wrappers_jni.h"

using base::android::AttachCurrentThread;
using base::android::ScopedJavaLocalRef;

namespace device {

ScopedJavaLocalRef<jobject> BluetoothAdapterWrapper_CreateWithDefaultAdapter() {
  return Java_BluetoothAdapterWrapper_createWithDefaultAdapter(
      AttachCurrentThread());
}

}  // namespace device

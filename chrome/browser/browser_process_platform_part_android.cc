// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/browser_process_platform_part_android.h"

#include "base/android/memory_pressure_listener_android.h"
#include "chrome/browser/lifetime/application_lifetime_android.h"

BrowserProcessPlatformPart::BrowserProcessPlatformPart() {
  base::android::MemoryPressureListenerAndroid::RegisterSystemCallback(
      base::android::AttachCurrentThread());
}

BrowserProcessPlatformPart::~BrowserProcessPlatformPart() {
}

void BrowserProcessPlatformPart::AttemptExit() {
  // Tell the Java code to finish() the Activity.
  chrome::TerminateAndroid();
}

// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ANDROID_LOCALE_UTILS_H_
#define BASE_ANDROID_LOCALE_UTILS_H_

#include <jni.h>

#include <string>

#include "base/base_export.h"

namespace base {
namespace android {

BASE_EXPORT std::string GetDefaultCountryCode();

// Return the current default locale of the device as string.
BASE_EXPORT std::string GetDefaultLocaleString();

}  // namespace android
}  // namespace base

#endif  // BASE_ANDROID_LOCALE_UTILS_H_

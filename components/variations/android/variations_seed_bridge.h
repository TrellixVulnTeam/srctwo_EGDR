// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_VARIATIONS_ANDROID_VARIATIONS_SEED_BRIDGE_H_
#define COMPONENTS_VARIATIONS_ANDROID_VARIATIONS_SEED_BRIDGE_H_

#include <jni.h>
#include <string>

namespace variations {
namespace android {

// Return the first run seed data pulled from the Java side of application.
void GetVariationsFirstRunSeed(std::string* seed_data,
                               std::string* seed_signature,
                               std::string* seed_country,
                               std::string* response_date,
                               bool* is_gzip_compressed);

// Clears first run seed preferences stored on the Java side of Chrome for
// Android.
void ClearJavaFirstRunPrefs();

// Marks variations seed as stored to avoid repeated fetches of the seed at
// the Java side.
void MarkVariationsSeedAsStored();

// Sets test data on the Java side. The data is pulled during the unit tests to
// C++ side and is being checked for consistency.
// This method is used for unit testing purposes only.
void SetJavaFirstRunPrefsForTesting(const std::string& seed_data,
                                    const std::string& seed_signature,
                                    const std::string& seed_country,
                                    const std::string& response_date,
                                    bool is_gzip_compressed);

}  // namespace android
}  // namespace variations

#endif  // COMPONENTS_VARIATIONS_ANDROID_FIRSTRUN_VARIATIONS_SEED_BRIDGE_H_

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/spellcheck/browser/android/component_jni_registrar.h"

#include "base/android/jni_android.h"
#include "base/android/jni_registrar.h"
#include "base/macros.h"
#include "components/spellcheck/browser/spellchecker_session_bridge_android.h"

namespace spellcheck {

namespace android {

static base::android::RegistrationMethod kSpellcheckRegisteredMethods[] = {
    {"SpellCheckerSessionBridge", SpellCheckerSessionBridge::RegisterJNI},
};

bool RegisterSpellcheckJni(JNIEnv* env) {
  return base::android::RegisterNativeMethods(
      env,
          kSpellcheckRegisteredMethods,
      arraysize(kSpellcheckRegisteredMethods));
}

}  // namespace android

}  // namespace spellcheck

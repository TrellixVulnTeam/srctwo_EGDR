// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <jni.h>

#include "base/android/jni_string.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/common/url_constants.h"
#include "jni/LoadUrlParams_jni.h"
#include "url/gurl.h"

using base::android::JavaParamRef;

namespace content {

jboolean IsDataScheme(JNIEnv* env,
                      const JavaParamRef<jclass>& clazz,
                      const JavaParamRef<jstring>& jurl) {
  GURL url(base::android::ConvertJavaStringToUTF8(env, jurl));
  return url.SchemeIs(url::kDataScheme);
}

}  // namespace content

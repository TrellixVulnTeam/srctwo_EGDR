// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "chrome/browser/search_engines/ui_thread_search_terms_data_android.h"
#include "jni/RevenueStats_jni.h"
#include "url/gurl.h"

using base::android::JavaParamRef;

namespace chrome {
namespace android {

static void SetSearchClient(JNIEnv* env,
                            const JavaParamRef<jclass>& clazz,
                            const JavaParamRef<jstring>& jclient) {
  SearchTermsDataAndroid::search_client_.Get() =
      base::android::ConvertJavaStringToUTF8(env, jclient);
}

static void SetRlzParameterValue(JNIEnv* env,
                                 const JavaParamRef<jclass>& clazz,
                                 const JavaParamRef<jstring>& jrlz) {
  SearchTermsDataAndroid::rlz_parameter_value_.Get() =
      base::android::ConvertJavaStringToUTF16(env, jrlz);
}

}  // namespace android
}  // namespace chrome

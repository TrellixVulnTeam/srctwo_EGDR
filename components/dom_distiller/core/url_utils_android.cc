// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/android/jni_string.h"
#include "components/dom_distiller/core/url_constants.h"
#include "components/dom_distiller/core/url_utils.h"
#include "jni/DomDistillerUrlUtils_jni.h"
#include "net/base/url_util.h"
#include "url/gurl.h"

using base::android::JavaParamRef;
using base::android::ScopedJavaLocalRef;

namespace dom_distiller {

namespace url_utils {

namespace android {

ScopedJavaLocalRef<jstring> GetDistillerViewUrlFromUrl(
    JNIEnv* env,
    const JavaParamRef<jclass>& clazz,
    const JavaParamRef<jstring>& j_scheme,
    const JavaParamRef<jstring>& j_url) {
  std::string scheme(base::android::ConvertJavaStringToUTF8(env, j_scheme));
  GURL url(base::android::ConvertJavaStringToUTF8(env, j_url));
  if (!url.is_valid()) {
    return ScopedJavaLocalRef<jstring>();
  }
  GURL view_url =
      dom_distiller::url_utils::GetDistillerViewUrlFromUrl(scheme, url);
  if (!view_url.is_valid()) {
    return ScopedJavaLocalRef<jstring>();
  }
  return base::android::ConvertUTF8ToJavaString(env, view_url.spec());
}

ScopedJavaLocalRef<jstring> GetOriginalUrlFromDistillerUrl(
    JNIEnv* env,
    const JavaParamRef<jclass>& clazz,
    const JavaParamRef<jstring>& j_url) {
  GURL url(base::android::ConvertJavaStringToUTF8(env, j_url));
  if (!url.is_valid())
    return ScopedJavaLocalRef<jstring>();

  GURL original_url =
      dom_distiller::url_utils::GetOriginalUrlFromDistillerUrl(url);
  if (!original_url.is_valid())
    return ScopedJavaLocalRef<jstring>();

  return base::android::ConvertUTF8ToJavaString(env, original_url.spec());
}

jboolean IsDistilledPage(JNIEnv* env,
                         const JavaParamRef<jclass>& clazz,
                         const JavaParamRef<jstring>& j_url) {
  GURL url(base::android::ConvertJavaStringToUTF8(env, j_url));
  return dom_distiller::url_utils::IsDistilledPage(url);
}

jboolean IsUrlDistillable(JNIEnv* env,
                          const JavaParamRef<jclass>& clazz,
                          const JavaParamRef<jstring>& j_url) {
  GURL url(base::android::ConvertJavaStringToUTF8(env, j_url));
  return dom_distiller::url_utils::IsUrlDistillable(url);
}

ScopedJavaLocalRef<jstring> GetIsDistillableJs(
    JNIEnv* env,
    const JavaParamRef<jclass>& clazz) {
  return base::android::ConvertUTF8ToJavaString(
      env, dom_distiller::url_utils::GetIsDistillableJs());
}

ScopedJavaLocalRef<jstring> GetValueForKeyInUrl(
    JNIEnv* env,
    const JavaParamRef<jclass>& clazz,
    const JavaParamRef<jstring>& j_url,
    const JavaParamRef<jstring>& j_key) {
  GURL url(base::android::ConvertJavaStringToUTF8(env, j_url));
  std::string key = base::android::ConvertJavaStringToUTF8(env, j_key);
  return base::android::ConvertUTF8ToJavaString(
      env, dom_distiller::url_utils::GetValueForKeyInUrl(url, key));
}

}  // namespace android

}  // namespace url_utils

}  // namespace dom_distiller

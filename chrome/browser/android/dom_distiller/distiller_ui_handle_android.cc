// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/android/dom_distiller/distiller_ui_handle_android.h"

#include "base/android/jni_string.h"
#include "chrome/browser/ui/android/view_android_helper.h"
#include "components/dom_distiller/core/url_utils.h"
#include "content/public/browser/web_contents.h"
#include "jni/DomDistillerUIUtils_jni.h"
#include "ui/android/window_android.h"
#include "url/gurl.h"

using base::android::ScopedJavaLocalRef;

namespace dom_distiller {

namespace android {

// static
void DistillerUIHandleAndroid::OpenSettings(
    content::WebContents* web_contents) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_DomDistillerUIUtils_openSettings(env,
                                        web_contents->GetJavaWebContents());
}

}  // namespace android

}  // namespace dom_distiller

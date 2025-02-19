// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ANDROID_FIND_IN_PAGE_FIND_IN_PAGE_BRIDGE_H_
#define CHROME_BROWSER_ANDROID_FIND_IN_PAGE_FIND_IN_PAGE_BRIDGE_H_

#include "base/android/jni_weak_ref.h"
#include "base/macros.h"
#include "content/public/browser/web_contents.h"

class FindInPageBridge {
 public:
  FindInPageBridge(JNIEnv* env,
                   const base::android::JavaRef<jobject>& obj,
                   const base::android::JavaRef<jobject>& j_web_contents);
  void Destroy(JNIEnv*, const base::android::JavaParamRef<jobject>&);

  void StartFinding(JNIEnv* env,
                    const base::android::JavaParamRef<jobject>& obj,
                    const base::android::JavaParamRef<jstring>& search_string,
                    jboolean forward_direction,
                    jboolean case_sensitive);

  void StopFinding(JNIEnv* env,
                   const base::android::JavaParamRef<jobject>& obj,
                   jboolean clearSelection);

  base::android::ScopedJavaLocalRef<jstring> GetPreviousFindText(
      JNIEnv* env,
      const base::android::JavaParamRef<jobject>& obj);

  void RequestFindMatchRects(JNIEnv* env,
                             const base::android::JavaParamRef<jobject>& obj,
                             jint current_version);

  void ActivateNearestFindResult(
      JNIEnv* env,
      const base::android::JavaParamRef<jobject>& obj,
      jfloat x,
      jfloat y);

  void ActivateFindInPageResultForAccessibility(
      JNIEnv* env,
      const base::android::JavaParamRef<jobject>& obj);

 private:
  content::WebContents* web_contents_;
  JavaObjectWeakGlobalRef weak_java_ref_;

  DISALLOW_COPY_AND_ASSIGN(FindInPageBridge);
};

#endif  // CHROME_BROWSER_ANDROID_FIND_IN_PAGE_FIND_IN_PAGE_BRIDGE_H_

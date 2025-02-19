// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ANDROID_WEBVIEW_BROWSER_AW_SETTINGS_H_
#define ANDROID_WEBVIEW_BROWSER_AW_SETTINGS_H_

#include <memory>

#include "base/android/jni_weak_ref.h"
#include "base/android/scoped_java_ref.h"
#include "content/public/browser/web_contents_observer.h"

namespace content {
struct WebPreferences;
}

namespace android_webview {

class AwRenderViewHostExt;

class AwSettings : public content::WebContentsObserver {
 public:
  static AwSettings* FromWebContents(content::WebContents* web_contents);

  AwSettings(JNIEnv* env, jobject obj, content::WebContents* web_contents);
  ~AwSettings() override;

  bool GetJavaScriptCanOpenWindowsAutomatically();

  // Called from Java. Methods with "Locked" suffix require that the settings
  // access lock is held during their execution.
  void Destroy(JNIEnv* env, const base::android::JavaParamRef<jobject>& obj);
  void PopulateWebPreferencesLocked(
      JNIEnv* env,
      const base::android::JavaParamRef<jobject>& obj,
      jlong web_prefs);
  void ResetScrollAndScaleState(
      JNIEnv* env,
      const base::android::JavaParamRef<jobject>& obj);
  void UpdateEverythingLocked(JNIEnv* env,
                              const base::android::JavaParamRef<jobject>& obj);
  void UpdateInitialPageScaleLocked(
      JNIEnv* env,
      const base::android::JavaParamRef<jobject>& obj);
  void UpdateUserAgentLocked(JNIEnv* env,
                             const base::android::JavaParamRef<jobject>& obj);
  void UpdateWebkitPreferencesLocked(
      JNIEnv* env,
      const base::android::JavaParamRef<jobject>& obj);
  void UpdateFormDataPreferencesLocked(
      JNIEnv* env,
      const base::android::JavaParamRef<jobject>& obj);
  void UpdateRendererPreferencesLocked(
      JNIEnv* env,
      const base::android::JavaParamRef<jobject>& obj);
  void UpdateOffscreenPreRasterLocked(
      JNIEnv* env,
      const base::android::JavaParamRef<jobject>& obj);

  void PopulateWebPreferences(content::WebPreferences* web_prefs);

 private:
  AwRenderViewHostExt* GetAwRenderViewHostExt();
  void UpdateEverything();

  // WebContentsObserver overrides:
  void RenderViewHostChanged(content::RenderViewHost* old_host,
                             content::RenderViewHost* new_host) override;
  void WebContentsDestroyed() override;

  bool renderer_prefs_initialized_;
  bool javascript_can_open_windows_automatically_;

  JavaObjectWeakGlobalRef aw_settings_;
};

}  // namespace android_webview

#endif  // ANDROID_WEBVIEW_BROWSER_AW_SETTINGS_H_

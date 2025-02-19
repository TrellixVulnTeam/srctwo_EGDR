// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/android/infobars/infobar_container_android.h"

#include "base/android/jni_android.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/metrics/histogram_macros.h"
#include "chrome/browser/infobars/infobar_service.h"
#include "chrome/browser/ui/android/infobars/infobar_android.h"
#include "components/infobars/core/infobar.h"
#include "components/infobars/core/infobar_delegate.h"
#include "content/public/browser/web_contents.h"
#include "jni/InfoBarContainer_jni.h"

using base::android::JavaParamRef;

// InfoBarContainerAndroid ----------------------------------------------------

InfoBarContainerAndroid::InfoBarContainerAndroid(JNIEnv* env,
                                                 jobject obj)
    : infobars::InfoBarContainer(NULL),
      weak_java_infobar_container_(env, obj) {}

InfoBarContainerAndroid::~InfoBarContainerAndroid() {
  RemoveAllInfoBarsForDestruction();
}

void InfoBarContainerAndroid::SetWebContents(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    const JavaParamRef<jobject>& web_contents) {
  InfoBarService* infobar_service = InfoBarService::FromWebContents(
      content::WebContents::FromJavaWebContents(web_contents));
  ChangeInfoBarManager(infobar_service);
}

void InfoBarContainerAndroid::Destroy(JNIEnv* env,
                                      const JavaParamRef<jobject>& obj) {
  delete this;
}

void InfoBarContainerAndroid::PlatformSpecificAddInfoBar(
    infobars::InfoBar* infobar,
    size_t position) {
  DCHECK(infobar);
  InfoBarAndroid* android_bar = static_cast<InfoBarAndroid*>(infobar);
  if (!android_bar) {
    // TODO(bulach): CLANK: implement other types of InfoBars.
    // TODO(jrg): this will always print out WARNING_TYPE as an int.
    // Try and be more helpful.
    NOTIMPLEMENTED() << "CLANK: infobar type "
                     << infobar->delegate()->GetInfoBarType();
    return;
  }

  AttachJavaInfoBar(android_bar);
}

void InfoBarContainerAndroid::AttachJavaInfoBar(InfoBarAndroid* android_bar) {
  if (android_bar->HasSetJavaInfoBar())
    return;
  JNIEnv* env = base::android::AttachCurrentThread();

  if (Java_InfoBarContainer_hasInfoBars(
          env, weak_java_infobar_container_.get(env))) {
    UMA_HISTOGRAM_SPARSE_SLOWLY("InfoBar.Shown.Hidden",
                                android_bar->delegate()->GetIdentifier());
    uintptr_t native_ptr = Java_InfoBarContainer_getTopNativeInfoBarPtr(
        env, weak_java_infobar_container_.get(env));
    if (native_ptr) {
      UMA_HISTOGRAM_SPARSE_SLOWLY("InfoBar.Shown.Hiding",
                                  reinterpret_cast<InfoBarAndroid*>(native_ptr)
                                      ->delegate()
                                      ->GetIdentifier());
    }
  } else {
    UMA_HISTOGRAM_SPARSE_SLOWLY("InfoBar.Shown.Visible",
                                android_bar->delegate()->GetIdentifier());
  }

  base::android::ScopedJavaLocalRef<jobject> java_infobar =
      android_bar->CreateRenderInfoBar(env);
  Java_InfoBarContainer_addInfoBar(env, weak_java_infobar_container_.get(env),
                                   java_infobar);
  android_bar->SetJavaInfoBar(java_infobar);
}

void InfoBarContainerAndroid::PlatformSpecificReplaceInfoBar(
    infobars::InfoBar* old_infobar,
    infobars::InfoBar* new_infobar) {
  static_cast<InfoBarAndroid*>(new_infobar)->PassJavaInfoBar(
      static_cast<InfoBarAndroid*>(old_infobar));
}

void InfoBarContainerAndroid::PlatformSpecificRemoveInfoBar(
    infobars::InfoBar* infobar) {
  InfoBarAndroid* android_infobar = static_cast<InfoBarAndroid*>(infobar);
  android_infobar->CloseJavaInfoBar();
}


// Native JNI methods ---------------------------------------------------------

static jlong Init(JNIEnv* env, const JavaParamRef<jobject>& obj) {
  InfoBarContainerAndroid* infobar_container =
      new InfoBarContainerAndroid(env, obj);
  return reinterpret_cast<intptr_t>(infobar_container);
}

// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_ANDROID_SMART_SELECTION_CLIENT_H_
#define CONTENT_BROWSER_ANDROID_SMART_SELECTION_CLIENT_H_

#include <jni.h>

#include "base/android/jni_weak_ref.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/string16.h"

namespace content {

class WebContents;

// The native portion of Java SmartSelectionClient.
// This class performs one operation: retrieves the selected text together with
// its surrounding context from WebContents. This surrounding text is obtained
// asynchronously from the current focused frame and passed back to Java layer.
class SmartSelectionClient {
 public:
  SmartSelectionClient(JNIEnv* env,
                       const base::android::JavaRef<jobject>& obj,
                       WebContents* web_contents);
  ~SmartSelectionClient();

  // Sends asynchronius request to retrieve the text.
  void RequestSurroundingText(JNIEnv* env,
                              const base::android::JavaParamRef<jobject>& obj,
                              int num_extra_characters,
                              int callback_data);

  // Cancels all pending requests.
  void CancelAllRequests(JNIEnv* env,
                         const base::android::JavaParamRef<jobject>& obj);

 private:
  void OnSurroundingTextReceived(int callback_data,
                                 const base::string16& text,
                                 int start,
                                 int end);

  // A weak reference to the Java ContentSelectionClient object.
  JavaObjectWeakGlobalRef java_ref_;

  // WebContents is used to find the relevant RenderFrameHost that can send
  // the request for the text.
  WebContents* web_contents_;

  base::WeakPtrFactory<SmartSelectionClient> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(SmartSelectionClient);
};

}  // namespace content

#endif  // CONTENT_BROWSER_ANDROID_SMART_SELECTION_CLIENT_H_

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/client/jni/jni_runtime_delegate.h"

#include "base/android/jni_android.h"
#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/android/library_loader/library_loader_hooks.h"
#include "base/android/scoped_java_ref.h"
#include "base/command_line.h"
#include "base/memory/ptr_util.h"
#include "base/memory/singleton.h"
#include "base/stl_util.h"
#include "base/synchronization/waitable_event.h"
#include "base/task_scheduler/task_scheduler.h"
#include "jni/JniInterface_jni.h"
#include "remoting/base/chromium_url_request.h"
#include "remoting/base/url_request_context_getter.h"
#include "remoting/client/jni/jni_touch_event_data.h"

using base::android::ConvertJavaStringToUTF8;
using base::android::ConvertUTF8ToJavaString;
using base::android::JavaParamRef;
using base::android::ToJavaByteArray;

namespace remoting {

// Implementation of stubs defined in JniInterface_jni.h. These are the entry
// points for JNI calls from Java into C++.

static void LoadNative(JNIEnv* env, const JavaParamRef<jclass>& clazz) {
  base::CommandLine::Init(0, nullptr);

  // Create the singleton now so that the Chromoting threads will be set up.
  ChromotingClientRuntime* runtime =
      remoting::ChromotingClientRuntime::GetInstance();
  JniRuntimeDelegate* delegate = remoting::JniRuntimeDelegate::GetInstance();
  runtime->SetDelegate(delegate);
}

static void HandleAuthTokenOnNetworkThread(const std::string& token) {
  ChromotingClientRuntime* runtime =
      remoting::ChromotingClientRuntime::GetInstance();
  DCHECK(runtime->network_task_runner()->BelongsToCurrentThread());
  runtime->log_writer()->SetAuthToken(token);
}

static void OnAuthTokenFetched(JNIEnv* env,
                               const JavaParamRef<jclass>& clazz,
                               const JavaParamRef<jstring>& token) {
  ChromotingClientRuntime* runtime =
      remoting::ChromotingClientRuntime::GetInstance();
  runtime->network_task_runner()->PostTask(
      FROM_HERE, base::Bind(&HandleAuthTokenOnNetworkThread,
                            ConvertJavaStringToUTF8(env, token)));
}

// JniRuntimeDelegate implementation.

// static
JniRuntimeDelegate* JniRuntimeDelegate::GetInstance() {
  return base::Singleton<JniRuntimeDelegate>::get();
}

JniRuntimeDelegate::JniRuntimeDelegate() {
  runtime_ = ChromotingClientRuntime::GetInstance();
}

JniRuntimeDelegate::~JniRuntimeDelegate() {
  runtime_ = nullptr;
}

void JniRuntimeDelegate::RuntimeWillShutdown() {
  DCHECK(runtime_->ui_task_runner()->BelongsToCurrentThread());

  base::WaitableEvent done_event(
      base::WaitableEvent::ResetPolicy::AUTOMATIC,
      base::WaitableEvent::InitialState::NOT_SIGNALED);
  runtime_->network_task_runner()->PostTask(
      FROM_HERE, base::Bind(&JniRuntimeDelegate::DetachFromVmAndSignal,
                            base::Unretained(this), &done_event));
  done_event.Wait();
  runtime_->display_task_runner()->PostTask(
      FROM_HERE, base::Bind(&JniRuntimeDelegate::DetachFromVmAndSignal,
                            base::Unretained(this), &done_event));
  done_event.Wait();
}

void JniRuntimeDelegate::RuntimeDidShutdown() {
  base::android::LibraryLoaderExitHook();
  base::android::DetachFromVM();
}

void JniRuntimeDelegate::RequestAuthTokenForLogger() {
  if (!runtime_->ui_task_runner()->BelongsToCurrentThread()) {
    runtime_->ui_task_runner()->PostTask(
        FROM_HERE, base::Bind(&JniRuntimeDelegate::RequestAuthTokenForLogger,
                              base::Unretained(this)));
    return;
  }
  JNIEnv* env = base::android::AttachCurrentThread();

  // TODO(nicholss): I do not like this method name, change it soon.
  Java_JniInterface_fetchAuthToken(env);
}

void JniRuntimeDelegate::DetachFromVmAndSignal(base::WaitableEvent* waiter) {
  base::android::DetachFromVM();
  waiter->Signal();
}

}  // namespace remoting

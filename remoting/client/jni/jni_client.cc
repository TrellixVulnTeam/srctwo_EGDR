// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/client/jni/jni_client.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/logging.h"
#include "jni/Client_jni.h"
#include "remoting/client/audio/audio_player_android.h"
#include "remoting/client/chromoting_client_runtime.h"
#include "remoting/client/chromoting_session.h"
#include "remoting/client/connect_to_host_info.h"
#include "remoting/client/jni/jni_gl_display_handler.h"
#include "remoting/client/jni/jni_pairing_secret_fetcher.h"
#include "remoting/client/jni/jni_runtime_delegate.h"
#include "remoting/client/jni/jni_touch_event_data.h"
#include "remoting/protocol/video_renderer.h"

using base::android::ConvertJavaStringToUTF8;
using base::android::ConvertUTF8ToJavaString;
using base::android::JavaParamRef;
using base::android::ScopedJavaLocalRef;

namespace remoting {

JniClient::JniClient(base::android::ScopedJavaGlobalRef<jobject> java_client)
    : java_client_(java_client), weak_factory_(this) {
  runtime_ = ChromotingClientRuntime::GetInstance();
  weak_ptr_ = weak_factory_.GetWeakPtr();
}

JniClient::~JniClient() {
  DCHECK(runtime_->ui_task_runner()->BelongsToCurrentThread());

  // The session must be shut down first, since it depends on our other
  // components' still being alive.
  DisconnectFromHost();
}

void JniClient::ConnectToHost(const ConnectToHostInfo& info) {
  DCHECK(runtime_->ui_task_runner()->BelongsToCurrentThread());
  DCHECK(!display_handler_);
  DCHECK(!audio_player_);
  DCHECK(!session_);
  DCHECK(!secret_fetcher_);
  display_handler_.reset(new JniGlDisplayHandler(java_client_));
  secret_fetcher_.reset(
      new JniPairingSecretFetcher(GetWeakPtr(), info.host_id));

  protocol::ClientAuthenticationConfig client_auth_config;
  client_auth_config.host_id = info.host_id;
  client_auth_config.pairing_client_id = info.pairing_id;
  client_auth_config.pairing_secret = info.pairing_secret;
  client_auth_config.fetch_secret_callback = base::Bind(
      &JniPairingSecretFetcher::FetchSecret, secret_fetcher_->GetWeakPtr());

  audio_player_.reset(new AudioPlayerAndroid());

  session_.reset(new ChromotingSession(
      weak_ptr_, display_handler_->CreateCursorShapeStub(),
      display_handler_->CreateVideoRenderer(), audio_player_->GetWeakPtr(),
      info, client_auth_config));
  session_->Connect();
}

void JniClient::DisconnectFromHost() {
  DCHECK(runtime_->ui_task_runner()->BelongsToCurrentThread());
  if (session_) {
    session_->Disconnect();
    runtime_->network_task_runner()->DeleteSoon(FROM_HERE,
                                                session_.release());
  }
  if (secret_fetcher_) {
    runtime_->network_task_runner()->DeleteSoon(FROM_HERE,
                                                secret_fetcher_.release());
  }
  if (audio_player_) {
    runtime_->network_task_runner()->DeleteSoon(FROM_HERE,
                                                audio_player_.release());
  }
  display_handler_.reset();
}

void JniClient::OnConnectionState(protocol::ConnectionToHost::State state,
                                  protocol::ErrorCode error) {
  DCHECK(runtime_->ui_task_runner()->BelongsToCurrentThread());

  JNIEnv* env = base::android::AttachCurrentThread();
  Java_Client_onConnectionState(env, java_client_, state, error);
}

void JniClient::DisplayAuthenticationPrompt(bool pairing_supported) {
  DCHECK(runtime_->ui_task_runner()->BelongsToCurrentThread());

  JNIEnv* env = base::android::AttachCurrentThread();
  Java_Client_displayAuthenticationPrompt(env, java_client_, pairing_supported);
}

void JniClient::CommitPairingCredentials(const std::string& host,
                                         const std::string& id,
                                         const std::string& secret) {
  DCHECK(runtime_->ui_task_runner()->BelongsToCurrentThread());

  JNIEnv* env = base::android::AttachCurrentThread();
  ScopedJavaLocalRef<jstring> j_host = ConvertUTF8ToJavaString(env, host);
  ScopedJavaLocalRef<jstring> j_id = ConvertUTF8ToJavaString(env, id);
  ScopedJavaLocalRef<jstring> j_secret = ConvertUTF8ToJavaString(env, secret);

  Java_Client_commitPairingCredentials(env, java_client_, j_host, j_id,
                                       j_secret);
}

void JniClient::FetchThirdPartyToken(const std::string& token_url,
                                     const std::string& client_id,
                                     const std::string& scope) {
  DCHECK(runtime_->ui_task_runner()->BelongsToCurrentThread());
  JNIEnv* env = base::android::AttachCurrentThread();

  ScopedJavaLocalRef<jstring> j_url = ConvertUTF8ToJavaString(env, token_url);
  ScopedJavaLocalRef<jstring> j_client_id =
      ConvertUTF8ToJavaString(env, client_id);
  ScopedJavaLocalRef<jstring> j_scope = ConvertUTF8ToJavaString(env, scope);

  Java_Client_fetchThirdPartyToken(env, java_client_, j_url, j_client_id,
                                   j_scope);
}

void JniClient::SetCapabilities(const std::string& capabilities) {
  DCHECK(runtime_->ui_task_runner()->BelongsToCurrentThread());
  JNIEnv* env = base::android::AttachCurrentThread();

  ScopedJavaLocalRef<jstring> j_cap =
      ConvertUTF8ToJavaString(env, capabilities);

  Java_Client_setCapabilities(env, java_client_, j_cap);
}

void JniClient::HandleExtensionMessage(const std::string& type,
                                       const std::string& message) {
  DCHECK(runtime_->ui_task_runner()->BelongsToCurrentThread());
  JNIEnv* env = base::android::AttachCurrentThread();

  ScopedJavaLocalRef<jstring> j_type = ConvertUTF8ToJavaString(env, type);
  ScopedJavaLocalRef<jstring> j_message = ConvertUTF8ToJavaString(env, message);

  Java_Client_handleExtensionMessage(env, java_client_, j_type, j_message);
}

void JniClient::Connect(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& caller,
    const base::android::JavaParamRef<jstring>& username,
    const base::android::JavaParamRef<jstring>& auth_token,
    const base::android::JavaParamRef<jstring>& host_jid,
    const base::android::JavaParamRef<jstring>& host_id,
    const base::android::JavaParamRef<jstring>& host_pubkey,
    const base::android::JavaParamRef<jstring>& pair_id,
    const base::android::JavaParamRef<jstring>& pair_secret,
    const base::android::JavaParamRef<jstring>& capabilities,
    const base::android::JavaParamRef<jstring>& flags,
    const base::android::JavaParamRef<jstring>& host_version,
    const base::android::JavaParamRef<jstring>& host_os,
    const base::android::JavaParamRef<jstring>& host_os_version) {
  ConnectToHostInfo info;
  info.username = ConvertJavaStringToUTF8(env, username);
  info.auth_token = ConvertJavaStringToUTF8(env, auth_token);
  info.host_jid = ConvertJavaStringToUTF8(env, host_jid);
  info.host_id = ConvertJavaStringToUTF8(env, host_id);
  info.host_pubkey = ConvertJavaStringToUTF8(env, host_pubkey);
  info.pairing_id = ConvertJavaStringToUTF8(env, pair_id);
  info.pairing_secret = ConvertJavaStringToUTF8(env, pair_secret);
  info.capabilities = ConvertJavaStringToUTF8(env, capabilities);
  info.flags = ConvertJavaStringToUTF8(env, flags);
  info.host_version = ConvertJavaStringToUTF8(env, host_version);
  info.host_os = ConvertJavaStringToUTF8(env, host_os);
  info.host_os_version = ConvertJavaStringToUTF8(env, host_os_version);

  ConnectToHost(info);
}

void JniClient::Disconnect(JNIEnv* env,
                           const base::android::JavaParamRef<jobject>& caller) {
  DisconnectFromHost();
}

void JniClient::AuthenticationResponse(
    JNIEnv* env,
    const JavaParamRef<jobject>& caller,
    const JavaParamRef<jstring>& pin,
    jboolean createPair,
    const JavaParamRef<jstring>& deviceName) {
  if (session_) {
    session_->ProvideSecret(ConvertJavaStringToUTF8(env, pin), createPair,
                            ConvertJavaStringToUTF8(env, deviceName));
  }

  if (secret_fetcher_) {
    runtime_->network_task_runner()->PostTask(
        FROM_HERE, base::Bind(&JniPairingSecretFetcher::ProvideSecret,
                              secret_fetcher_->GetWeakPtr(),
                              ConvertJavaStringToUTF8(env, pin)));
  }
}

void JniClient::SendMouseEvent(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& caller,
    jint x,
    jint y,
    jint whichButton,
    jboolean buttonDown) {
  // Button must be within the bounds of the MouseEvent_MouseButton enum.
  DCHECK(whichButton >= 0 && whichButton < 5);

  session_->SendMouseEvent(
      x, y,
      static_cast<remoting::protocol::MouseEvent_MouseButton>(whichButton),
      buttonDown);
}

void JniClient::SendMouseWheelEvent(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& caller,
    jint delta_x,
    jint delta_y) {
  session_->SendMouseWheelEvent(delta_x, delta_y);
}

jboolean JniClient::SendKeyEvent(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& caller,
    jint scanCode,
    jint keyCode,
    jboolean keyDown) {
  return session_->SendKeyEvent(scanCode, keyCode, keyDown);
}

void JniClient::SendTextEvent(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& caller,
    const JavaParamRef<jstring>& text) {
  session_->SendTextEvent(ConvertJavaStringToUTF8(env, text));
}

void JniClient::SendTouchEvent(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& caller,
    jint eventType,
    const JavaParamRef<jobjectArray>& touchEventObjectArray) {
  protocol::TouchEvent touch_event;
  touch_event.set_event_type(
      static_cast<protocol::TouchEvent::TouchEventType>(eventType));

  // Iterate over the elements in the object array and transfer the data from
  // the java object to a native event object.
  jsize length = env->GetArrayLength(touchEventObjectArray);
  DCHECK_GE(length, 0);
  for (jsize i = 0; i < length; ++i) {
    protocol::TouchEventPoint* touch_point = touch_event.add_touch_points();

    ScopedJavaLocalRef<jobject> java_touch_event(
        env, env->GetObjectArrayElement(touchEventObjectArray, i));

    JniTouchEventData::CopyTouchPointData(env, java_touch_event, touch_point);
  }

  session_->SendTouchEvent(touch_event);
}

void JniClient::EnableVideoChannel(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& caller,
    jboolean enable) {
  session_->EnableVideoChannel(enable);
}

void JniClient::OnThirdPartyTokenFetched(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& caller,
    const JavaParamRef<jstring>& token,
    const JavaParamRef<jstring>& shared_secret) {
  runtime_->network_task_runner()->PostTask(
      FROM_HERE,
      base::Bind(&ChromotingSession::HandleOnThirdPartyTokenFetched,
                 session_->GetWeakPtr(), ConvertJavaStringToUTF8(env, token),
                 ConvertJavaStringToUTF8(env, shared_secret)));
}

void JniClient::SendExtensionMessage(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& caller,
    const JavaParamRef<jstring>& type,
    const JavaParamRef<jstring>& data) {
  session_->SendClientMessage(ConvertJavaStringToUTF8(env, type),
                              ConvertJavaStringToUTF8(env, data));
}

void JniClient::Destroy(JNIEnv* env, const JavaParamRef<jobject>& caller) {
  delete this;
}

base::WeakPtr<JniClient> JniClient::GetWeakPtr() {
  return weak_ptr_;
}

static jlong Init(JNIEnv* env, const JavaParamRef<jobject>& caller) {
  return reinterpret_cast<intptr_t>(
      new JniClient(base::android::ScopedJavaGlobalRef<jobject>(env, caller)));
}

}  // namespace remoting

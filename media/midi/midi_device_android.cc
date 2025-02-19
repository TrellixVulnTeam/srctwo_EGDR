// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/midi/midi_device_android.h"

#include <string>

#include "base/android/jni_string.h"
#include "base/memory/ptr_util.h"
#include "jni/MidiDeviceAndroid_jni.h"
#include "media/midi/midi_output_port_android.h"

using base::android::JavaRef;
using base::android::ScopedJavaLocalRef;

namespace midi {

namespace {

std::string ConvertMaybeJavaString(JNIEnv* env,
                                   const base::android::JavaRef<jstring>& str) {
  if (!str.obj())
    return std::string();
  return base::android::ConvertJavaStringToUTF8(str);
}
}

MidiDeviceAndroid::MidiDeviceAndroid(JNIEnv* env,
                                     const JavaRef<jobject>& raw_device,
                                     MidiInputPortAndroid::Delegate* delegate)
    : raw_device_(raw_device) {
  ScopedJavaLocalRef<jobjectArray> raw_input_ports =
      Java_MidiDeviceAndroid_getInputPorts(env, raw_device);
  jsize num_input_ports = env->GetArrayLength(raw_input_ports.obj());

  for (jsize i = 0; i < num_input_ports; ++i) {
    jobject port = env->GetObjectArrayElement(raw_input_ports.obj(), i);
    input_ports_.push_back(
        base::MakeUnique<MidiInputPortAndroid>(env, port, delegate));
  }

  ScopedJavaLocalRef<jobjectArray> raw_output_ports =
      Java_MidiDeviceAndroid_getOutputPorts(env, raw_device);
  jsize num_output_ports = env->GetArrayLength(raw_output_ports.obj());
  for (jsize i = 0; i < num_output_ports; ++i) {
    jobject port = env->GetObjectArrayElement(raw_output_ports.obj(), i);
    output_ports_.push_back(base::MakeUnique<MidiOutputPortAndroid>(env, port));
  }
}

MidiDeviceAndroid::~MidiDeviceAndroid() {}

std::string MidiDeviceAndroid::GetManufacturer() {
  JNIEnv* env = base::android::AttachCurrentThread();
  return ConvertMaybeJavaString(
      env, Java_MidiDeviceAndroid_getManufacturer(env, raw_device_));
}

std::string MidiDeviceAndroid::GetProductName() {
  JNIEnv* env = base::android::AttachCurrentThread();
  return ConvertMaybeJavaString(
      env, Java_MidiDeviceAndroid_getProduct(env, raw_device_));
}

std::string MidiDeviceAndroid::GetDeviceVersion() {
  JNIEnv* env = base::android::AttachCurrentThread();
  return ConvertMaybeJavaString(
      env, Java_MidiDeviceAndroid_getVersion(env, raw_device_));
}

}  // namespace midi

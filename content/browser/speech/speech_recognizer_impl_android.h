// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_SPEECH_SPEECH_RECOGNIZER_IMPL_ANDROID_H_
#define CONTENT_BROWSER_SPEECH_SPEECH_RECOGNIZER_IMPL_ANDROID_H_

#include <jni.h>

#include <memory>

#include "base/android/scoped_java_ref.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "content/browser/speech/speech_recognizer.h"
#include "content/public/common/speech_recognition_error.h"
#include "content/public/common/speech_recognition_result.h"

namespace content {

class SpeechRecognitionEventListener;

class CONTENT_EXPORT SpeechRecognizerImplAndroid : public SpeechRecognizer {
 public:
  SpeechRecognizerImplAndroid(SpeechRecognitionEventListener* listener,
                              int session_id);

  // SpeechRecognizer methods.
  void StartRecognition(const std::string& device_id) override;
  void AbortRecognition() override;
  void StopAudioCapture() override;
  bool IsActive() const override;
  bool IsCapturingAudio() const override;

  // Called from Java methods via JNI.
  void OnAudioStart(JNIEnv* env,
                    const base::android::JavaParamRef<jobject>& obj);
  void OnSoundStart(JNIEnv* env,
                    const base::android::JavaParamRef<jobject>& obj);
  void OnSoundEnd(JNIEnv* env, const base::android::JavaParamRef<jobject>& obj);
  void OnAudioEnd(JNIEnv* env, const base::android::JavaParamRef<jobject>& obj);
  void OnRecognitionResults(
      JNIEnv* env,
      const base::android::JavaParamRef<jobject>& obj,
      const base::android::JavaParamRef<jobjectArray>& strings,
      const base::android::JavaParamRef<jfloatArray>& floats,
      jboolean interim);
  void OnRecognitionError(JNIEnv* env,
                          const base::android::JavaParamRef<jobject>& obj,
                          jint error);
  void OnRecognitionEnd(JNIEnv* env,
                        const base::android::JavaParamRef<jobject>& obj);

 private:
  enum State {
    STATE_IDLE = 0,
    STATE_CAPTURING_AUDIO,
    STATE_AWAITING_FINAL_RESULT
  };

  void StartRecognitionOnUIThread(const std::string& language,
                                  bool continuous,
                                  bool interim_results);
  void OnRecognitionResultsOnIOThread(SpeechRecognitionResults const &results);

  ~SpeechRecognizerImplAndroid() override;

  base::android::ScopedJavaGlobalRef<jobject> j_recognition_;
  State state_;

  DISALLOW_COPY_AND_ASSIGN(SpeechRecognizerImplAndroid);
};

}  // namespace content

#endif  // CONTENT_BROWSER_SPEECH_SPEECH_RECOGNIZER_IMPL_ANDROID_H_

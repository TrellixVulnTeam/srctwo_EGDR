// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/app_list/speech_recognizer.h"

#include <stddef.h>
#include <stdint.h>

#include <algorithm>

#include "base/bind.h"
#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/timer/timer.h"
#include "chrome/browser/ui/app_list/speech_recognizer_delegate.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/speech_recognition_event_listener.h"
#include "content/public/browser/speech_recognition_manager.h"
#include "content/public/browser/speech_recognition_session_config.h"
#include "content/public/browser/speech_recognition_session_preamble.h"
#include "content/public/common/child_process_host.h"
#include "content/public/common/speech_recognition_error.h"
#include "net/url_request/url_request_context_getter.h"
#include "ui/app_list/speech_ui_model_observer.h"

namespace app_list {

// Length of timeout to cancel recognition if there's no speech heard.
static const int kNoSpeechTimeoutInSeconds = 5;

// Length of timeout to cancel recognition if no different results are received.
static const int kNoNewSpeechTimeoutInSeconds = 2;

// Invalid speech session.
static const int kInvalidSessionId = -1;

// Speech recognizer listener. This is separate from SpeechRecognizer because
// the speech recognition engine must function from the IO thread. Because of
// this, the lifecycle of this class must be decoupled from the lifecycle of
// SpeechRecognizer. To avoid circular references, this class has no reference
// to SpeechRecognizer. Instead, it has a reference to the
// SpeechRecognizerDelegate via a weak pointer that is only ever referenced from
// the UI thread.
class SpeechRecognizer::EventListener
    : public base::RefCountedThreadSafe<SpeechRecognizer::EventListener>,
      public content::SpeechRecognitionEventListener {
 public:
  EventListener(const base::WeakPtr<SpeechRecognizerDelegate>& delegate,
                net::URLRequestContextGetter* url_request_context_getter,
                const std::string& locale);

  void StartOnIOThread(
      const std::string& auth_scope,
      const std::string& auth_token,
      const scoped_refptr<content::SpeechRecognitionSessionPreamble>& preamble);
  void StopOnIOThread();

 private:
  friend class base::RefCountedThreadSafe<SpeechRecognizer::EventListener>;
  ~EventListener() override;

  void NotifyRecognitionStateChanged(SpeechRecognitionState new_state);

  // Starts a timer for |timeout_seconds|. When the timer expires, will stop
  // capturing audio and get a final utterance from the recognition manager.
  void StartSpeechTimeout(int timeout_seconds);
  void StopSpeechTimeout();
  void SpeechTimeout();

  // Overidden from content::SpeechRecognitionEventListener:
  // These are always called on the IO thread.
  void OnRecognitionStart(int session_id) override;
  void OnRecognitionEnd(int session_id) override;
  void OnRecognitionResults(
      int session_id,
      const content::SpeechRecognitionResults& results) override;
  void OnRecognitionError(
      int session_id, const content::SpeechRecognitionError& error) override;
  void OnSoundStart(int session_id) override;
  void OnSoundEnd(int session_id) override;
  void OnAudioLevelsChange(
      int session_id, float volume, float noise_volume) override;
  void OnEnvironmentEstimationComplete(int session_id) override;
  void OnAudioStart(int session_id) override;
  void OnAudioEnd(int session_id) override;

  // Only dereferenced from the UI thread, but copied on IO thread.
  base::WeakPtr<SpeechRecognizerDelegate> delegate_;

  // All remaining members only accessed from the IO thread.
  scoped_refptr<net::URLRequestContextGetter> url_request_context_getter_;
  std::string locale_;
  base::Timer speech_timeout_;
  int session_;
  base::string16 last_result_str_;

  base::WeakPtrFactory<EventListener> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(EventListener);
};

SpeechRecognizer::EventListener::EventListener(
    const base::WeakPtr<SpeechRecognizerDelegate>& delegate,
    net::URLRequestContextGetter* url_request_context_getter,
    const std::string& locale)
    : delegate_(delegate),
      url_request_context_getter_(url_request_context_getter),
      locale_(locale),
      speech_timeout_(false, false),
      session_(kInvalidSessionId),
      weak_factory_(this) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
}

SpeechRecognizer::EventListener::~EventListener() {
  DCHECK(!speech_timeout_.IsRunning());
}

void SpeechRecognizer::EventListener::StartOnIOThread(
    const std::string& auth_scope,
    const std::string& auth_token,
    const scoped_refptr<content::SpeechRecognitionSessionPreamble>& preamble) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
  if (session_ != kInvalidSessionId)
    StopOnIOThread();

  content::SpeechRecognitionSessionConfig config;
  config.language = locale_;
  config.continuous = true;
  config.interim_results = true;
  config.max_hypotheses = 1;
  config.filter_profanities = true;
  config.url_request_context_getter = url_request_context_getter_;
  config.event_listener = weak_factory_.GetWeakPtr();
  // kInvalidUniqueID is not a valid render process, so the speech permission
  // check allows the request through.
  config.initial_context.render_process_id =
      content::ChildProcessHost::kInvalidUniqueID;
  config.auth_scope = auth_scope;
  config.auth_token = auth_token;
  config.preamble = preamble;

  auto* speech_instance = content::SpeechRecognitionManager::GetInstance();
  session_ = speech_instance->CreateSession(config);
  speech_instance->StartSession(session_);
}

void SpeechRecognizer::EventListener::StopOnIOThread() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
  if (session_ == kInvalidSessionId)
    return;

  // Prevent recursion.
  int session = session_;
  session_ = kInvalidSessionId;
  StopSpeechTimeout();
  content::SpeechRecognitionManager::GetInstance()->StopAudioCaptureForSession(
      session);
  weak_factory_.InvalidateWeakPtrs();
}

void SpeechRecognizer::EventListener::NotifyRecognitionStateChanged(
    SpeechRecognitionState new_state) {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&SpeechRecognizerDelegate::OnSpeechRecognitionStateChanged,
                 delegate_,
                 new_state));
}

void SpeechRecognizer::EventListener::StartSpeechTimeout(int timeout_seconds) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
  speech_timeout_.Start(
      FROM_HERE,
      base::TimeDelta::FromSeconds(timeout_seconds),
      base::Bind(&SpeechRecognizer::EventListener::SpeechTimeout, this));
}

void SpeechRecognizer::EventListener::StopSpeechTimeout() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
  speech_timeout_.Stop();
}

void SpeechRecognizer::EventListener::SpeechTimeout() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
  StopOnIOThread();
}

void SpeechRecognizer::EventListener::OnRecognitionStart(int session_id) {
  NotifyRecognitionStateChanged(SPEECH_RECOGNITION_RECOGNIZING);
}

void SpeechRecognizer::EventListener::OnRecognitionEnd(int session_id) {
  StopOnIOThread();
  NotifyRecognitionStateChanged(SPEECH_RECOGNITION_READY);
}

void SpeechRecognizer::EventListener::OnRecognitionResults(
    int session_id, const content::SpeechRecognitionResults& results) {
  base::string16 result_str;
  size_t final_count = 0;
  // The number of results with |is_provisional| false. If |final_count| ==
  // results.size(), then all results are non-provisional and the recognition is
  // complete.
  for (const auto& result : results) {
    if (!result.is_provisional)
      final_count++;
    result_str += result.hypotheses[0].utterance;
  }
  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&SpeechRecognizerDelegate::OnSpeechResult,
                 delegate_,
                 result_str,
                 final_count == results.size()));

  // Stop the moment we have a final result. If we receive any new or changed
  // text, restart the timer to give the user more time to speak. (The timer is
  // recording the amount of time since the most recent utterance.)
  if (final_count == results.size())
    StopOnIOThread();
  else if (result_str != last_result_str_)
    StartSpeechTimeout(kNoNewSpeechTimeoutInSeconds);

  last_result_str_ = result_str;
}

void SpeechRecognizer::EventListener::OnRecognitionError(
    int session_id, const content::SpeechRecognitionError& error) {
  StopOnIOThread();
  if (error.code == content::SPEECH_RECOGNITION_ERROR_NETWORK) {
    NotifyRecognitionStateChanged(SPEECH_RECOGNITION_NETWORK_ERROR);
  }
  NotifyRecognitionStateChanged(SPEECH_RECOGNITION_READY);
}

void SpeechRecognizer::EventListener::OnSoundStart(int session_id) {
  StartSpeechTimeout(kNoSpeechTimeoutInSeconds);
  NotifyRecognitionStateChanged(SPEECH_RECOGNITION_IN_SPEECH);
}

void SpeechRecognizer::EventListener::OnSoundEnd(int session_id) {
  StopOnIOThread();
  NotifyRecognitionStateChanged(SPEECH_RECOGNITION_RECOGNIZING);
}

void SpeechRecognizer::EventListener::OnAudioLevelsChange(
    int session_id, float volume, float noise_volume) {
  DCHECK_LE(0.0, volume);
  DCHECK_GE(1.0, volume);
  DCHECK_LE(0.0, noise_volume);
  DCHECK_GE(1.0, noise_volume);
  volume = std::max(0.0f, volume - noise_volume);
  // Both |volume| and |noise_volume| are defined to be in the range [0.0, 1.0].
  // See: content/public/browser/speech_recognition_event_listener.h
  int16_t sound_level = static_cast<int16_t>(INT16_MAX * volume);
  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&SpeechRecognizerDelegate::OnSpeechSoundLevelChanged,
                 delegate_,
                 sound_level));
}

void SpeechRecognizer::EventListener::OnEnvironmentEstimationComplete(
    int session_id) {
}

void SpeechRecognizer::EventListener::OnAudioStart(int session_id) {
}

void SpeechRecognizer::EventListener::OnAudioEnd(int session_id) {
}

SpeechRecognizer::SpeechRecognizer(
    const base::WeakPtr<SpeechRecognizerDelegate>& delegate,
    net::URLRequestContextGetter* url_request_context_getter,
    const std::string& locale)
    : delegate_(delegate),
      speech_event_listener_(new EventListener(
          delegate, url_request_context_getter, locale)) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
}

SpeechRecognizer::~SpeechRecognizer() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  Stop();
}

void SpeechRecognizer::Start(
    const scoped_refptr<content::SpeechRecognitionSessionPreamble>& preamble) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  std::string auth_scope;
  std::string auth_token;
  delegate_->GetSpeechAuthParameters(&auth_scope, &auth_token);

  content::BrowserThread::PostTask(
      content::BrowserThread::IO,
      FROM_HERE,
      base::Bind(&SpeechRecognizer::EventListener::StartOnIOThread,
                 speech_event_listener_,
                 auth_scope,
                 auth_token,
                 preamble));
}

void SpeechRecognizer::Stop() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  content::BrowserThread::PostTask(
      content::BrowserThread::IO,
      FROM_HERE,
      base::Bind(&SpeechRecognizer::EventListener::StopOnIOThread,
                 speech_event_listener_));
}

}  // namespace app_list

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_MEDIA_AUDIO_CAST_AUDIO_MIXER_H_
#define CHROMECAST_MEDIA_AUDIO_CAST_AUDIO_MIXER_H_

#include <memory>
#include <set>

#include "base/callback.h"
#include "base/macros.h"
#include "base/threading/thread_checker.h"
#include "base/time/time.h"
#include "media/audio/audio_io.h"
#include "media/base/audio_converter.h"
#include "media/base/audio_parameters.h"

namespace chromecast {
namespace media {

class CastAudioManager;

// CastAudioMixer mixes multiple AudioOutputStreams and passes the mixed
// stream down to a single AudioOutputStream to be rendered by the CMA backend.
class CastAudioMixer : public ::media::AudioOutputStream::AudioSourceCallback {
 public:
  explicit CastAudioMixer(CastAudioManager* audio_manager);
  ~CastAudioMixer() override;

  virtual ::media::AudioOutputStream* MakeStream(
      const ::media::AudioParameters& params);

 private:
  class MixerProxyStream;

  // ::media::AudioOutputStream::AudioSourceCallback implementation
  int OnMoreData(base::TimeDelta delay,
                 base::TimeTicks delay_timestamp,
                 int prior_frames_skipped,
                 ::media::AudioBus* dest) override;
  void OnError() override;

  // MixedAudioOutputStreams call Register on opening and AddInput on starting.
  bool Register(MixerProxyStream* proxy_stream);
  void Unregister(MixerProxyStream* proxy_stream);
  void AddInput(::media::AudioConverter::InputCallback* input_callback);
  void RemoveInput(::media::AudioConverter::InputCallback* input_callback);
  void HandleError();

  CastAudioManager* const audio_manager_;
  bool error_;
  std::set<MixerProxyStream*> proxy_streams_;
  std::unique_ptr<::media::AudioConverter> mixer_;
  base::Lock mixer_lock_;
  ::media::AudioParameters output_params_;
  ::media::AudioOutputStream* output_stream_;

  THREAD_CHECKER(audio_thread_checker_);
  DISALLOW_COPY_AND_ASSIGN(CastAudioMixer);
};

}  // namespace media
}  // namespace chromecast

#endif  // CHROMECAST_MEDIA_AUDIO_CAST_AUDIO_MIXER_H_

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_BASE_AUDIO_RENDERER_MIXER_H_
#define MEDIA_BASE_AUDIO_RENDERER_MIXER_H_

#include <stdint.h>

#include <map>
#include <memory>
#include <string>

#include "base/macros.h"
#include "base/synchronization/lock.h"
#include "base/time/time.h"
#include "media/base/audio_converter.h"
#include "media/base/audio_renderer_sink.h"
#include "media/base/loopback_audio_converter.h"

namespace media {

// Mixes a set of AudioConverter::InputCallbacks into a single output stream
// which is funneled into a single shared AudioRendererSink; saving a bundle
// on renderer side resources.
class MEDIA_EXPORT AudioRendererMixer
    : public AudioRendererSink::RenderCallback {
 public:
  typedef base::Callback<void(int)> UmaLogCallback;

  AudioRendererMixer(const AudioParameters& output_params,
                     scoped_refptr<AudioRendererSink> sink,
                     const UmaLogCallback& log_callback);
  ~AudioRendererMixer() override;

  // Add or remove a mixer input from mixing; called by AudioRendererMixerInput.
  void AddMixerInput(const AudioParameters& input_params,
                     AudioConverter::InputCallback* input);
  void RemoveMixerInput(const AudioParameters& input_params,
                        AudioConverter::InputCallback* input);

  // Since errors may occur even when no inputs are playing, an error callback
  // must be registered separately from adding a mixer input.  The same callback
  // must be given to both the functions.
  void AddErrorCallback(const base::Closure& error_cb);
  void RemoveErrorCallback(const base::Closure& error_cb);

  void set_pause_delay_for_testing(base::TimeDelta delay) {
    pause_delay_ = delay;
  }

  OutputDeviceInfo GetOutputDeviceInfo();

  // Returns true if called on rendering thread, otherwise false.
  bool CurrentThreadIsRenderingThread();

  const AudioParameters& GetOutputParamsForTesting() { return output_params_; };

 private:
  class UMAMaxValueTracker;

  // Maps input sample rate to the dedicated converter.
  using AudioConvertersMap =
      std::map<int, std::unique_ptr<LoopbackAudioConverter>>;

  // AudioRendererSink::RenderCallback implementation.
  int Render(base::TimeDelta delay,
             base::TimeTicks delay_timestamp,
             int prior_frames_skipped,
             AudioBus* audio_bus) override;
  void OnRenderError() override;

  bool is_master_sample_rate(int sample_rate) {
    return sample_rate == output_params_.sample_rate();
  }

  // Output parameters for this mixer.
  const AudioParameters output_params_;

  // Output sink for this mixer.
  const scoped_refptr<AudioRendererSink> audio_sink_;

  // ---------------[ All variables below protected by |lock_| ]---------------
  base::Lock lock_;

  // List of error callbacks used by this mixer.
  typedef std::list<base::Closure> ErrorCallbackList;
  ErrorCallbackList error_callbacks_;

  // Each of these converters mixes inputs with a given sample rate and
  // resamples them to the output sample rate. Inputs not reqiuring resampling
  // go directly to |master_converter_|.
  AudioConvertersMap converters_;

  // Master converter which mixes all the outputs from |converters_| as well as
  // mixer inputs that are in the output sample rate.
  AudioConverter master_converter_;

  // Handles physical stream pause when no inputs are playing.  For latency
  // reasons we don't want to immediately pause the physical stream.
  base::TimeDelta pause_delay_;
  base::TimeTicks last_play_time_;
  bool playing_;

  // Tracks the maximum number of simultaneous mixer inputs and logs it into
  // UMA histogram upon the destruction.
  std::unique_ptr<UMAMaxValueTracker> input_count_tracker_;

  DISALLOW_COPY_AND_ASSIGN(AudioRendererMixer);
};

}  // namespace media

#endif  // MEDIA_BASE_AUDIO_RENDERER_MIXER_H_

// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/audio/android/audio_track_output_stream.h"

#include <cmath>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/logging.h"
#include "base/single_thread_task_runner.h"
#include "base/time/default_tick_clock.h"
#include "jni/AudioTrackOutputStream_jni.h"
#include "media/audio/audio_manager_base.h"
#include "media/base/audio_sample_types.h"
#include "media/base/audio_timestamp_helper.h"

using base::android::AttachCurrentThread;
using base::android::ScopedJavaLocalRef;

namespace media {

// Android audio format. For more information, please see:
// https://developer.android.com/reference/android/media/AudioFormat.html
enum {
  kEncodingPcm16bit = 2,  // ENCODING_PCM_16BIT
  kEncodingAc3 = 5,       // ENCODING_AC3
  kEncodingEac3 = 6,      // ENCODING_E_AC3
};

AudioTrackOutputStream::AudioTrackOutputStream(AudioManagerBase* manager,
                                               const AudioParameters& params)
    : params_(params),
      audio_manager_(manager),
      tick_clock_(new base::DefaultTickClock()) {
  if (!params_.IsBitstreamFormat()) {
    audio_bus_ = AudioBus::Create(params_);
  }
}

AudioTrackOutputStream::~AudioTrackOutputStream() {
  DCHECK(!callback_);
}

bool AudioTrackOutputStream::Open() {
  DCHECK(audio_manager_->GetTaskRunner()->BelongsToCurrentThread());
  JNIEnv* env = AttachCurrentThread();
  j_audio_output_stream_.Reset(Java_AudioTrackOutputStream_create(env));

  int format = kEncodingPcm16bit;
  if (params_.IsBitstreamFormat()) {
    if (params_.format() == AudioParameters::AUDIO_BITSTREAM_AC3) {
      format = kEncodingAc3;
    } else if (params_.format() == AudioParameters::AUDIO_BITSTREAM_EAC3) {
      format = kEncodingEac3;
    } else {
      NOTREACHED();
    }
  }

  return Java_AudioTrackOutputStream_open(env, j_audio_output_stream_,
                                          params_.channels(),
                                          params_.sample_rate(), format);
}

void AudioTrackOutputStream::Start(AudioSourceCallback* callback) {
  DCHECK(audio_manager_->GetTaskRunner()->BelongsToCurrentThread());
  callback_ = callback;
  Java_AudioTrackOutputStream_start(AttachCurrentThread(),
                                    j_audio_output_stream_,
                                    reinterpret_cast<intptr_t>(this));
}

void AudioTrackOutputStream::Stop() {
  DCHECK(audio_manager_->GetTaskRunner()->BelongsToCurrentThread());
  Java_AudioTrackOutputStream_stop(AttachCurrentThread(),
                                   j_audio_output_stream_);
  callback_ = nullptr;
}

void AudioTrackOutputStream::Close() {
  DCHECK(!callback_);
  DCHECK(audio_manager_->GetTaskRunner()->BelongsToCurrentThread());

  Java_AudioTrackOutputStream_close(AttachCurrentThread(),
                                    j_audio_output_stream_);
  audio_manager_->ReleaseOutputStream(this);
}

void AudioTrackOutputStream::SetMute(bool muted) {
  if (params_.IsBitstreamFormat() && muted) {
    LOG(WARNING)
        << "Mute is not supported for compressed audio bitstream formats.";
    return;
  }

  if (muted_ == muted)
    return;

  muted_ = muted;
  Java_AudioTrackOutputStream_setVolume(
      AttachCurrentThread(), j_audio_output_stream_, muted_ ? 0.0 : volume_);
}

void AudioTrackOutputStream::SetVolume(double volume) {
  if (params_.IsBitstreamFormat()) {
    LOG(WARNING) << "Volume change is not supported for compressed audio "
                    "bitstream formats.";
    return;
  }

  // Track |volume_| since AudioTrack uses a scaled value.
  volume_ = volume;
  if (muted_)
    return;

  Java_AudioTrackOutputStream_setVolume(AttachCurrentThread(),
                                        j_audio_output_stream_, volume);
};

void AudioTrackOutputStream::GetVolume(double* volume) {
  *volume = volume_;
};

// AudioOutputStream::SourceCallback implementation methods called from Java.
ScopedJavaLocalRef<jobject> AudioTrackOutputStream::OnMoreData(
    JNIEnv* env,
    jobject obj,
    jobject audio_data,
    jlong delay_in_frame) {
  DCHECK(callback_);

  base::TimeDelta delay =
      AudioTimestampHelper::FramesToTime(delay_in_frame, params_.sample_rate());

  void* native_buffer = env->GetDirectBufferAddress(audio_data);

  if (params_.IsBitstreamFormat()) {
    // For bitstream formats, use the direct buffer memory to avoid additional
    // memory copy.
    std::unique_ptr<AudioBus> audio_bus(
        AudioBus::WrapMemory(params_, native_buffer));
    audio_bus->set_is_bitstream_format(true);

    callback_->OnMoreData(delay, tick_clock_->NowTicks(), 0, audio_bus.get());

    if (audio_bus->GetBitstreamDataSize() <= 0)
      return nullptr;

    return Java_AudioTrackOutputStream_createAudioBufferInfo(
        env, j_audio_output_stream_, audio_bus->GetBitstreamFrames(),
        audio_bus->GetBitstreamDataSize());
  }

  // For PCM format, we need extra memory to convert planar float32 into
  // interleaved int16.

  callback_->OnMoreData(delay, tick_clock_->NowTicks(), 0, audio_bus_.get());

  int16_t* native_bus = reinterpret_cast<int16_t*>(native_buffer);
  audio_bus_->ToInterleaved<SignedInt16SampleTypeTraits>(audio_bus_->frames(),
                                                         native_bus);

  return Java_AudioTrackOutputStream_createAudioBufferInfo(
      env, j_audio_output_stream_, audio_bus_->frames(),
      sizeof(*native_bus) * audio_bus_->channels() * audio_bus_->frames());
}

void AudioTrackOutputStream::OnError(JNIEnv* env, jobject obj) {
  DCHECK(callback_);
  callback_->OnError();
}

jlong AudioTrackOutputStream::GetAddress(JNIEnv* env,
                                         jobject obj,
                                         jobject byte_buffer) {
  return reinterpret_cast<jlong>(env->GetDirectBufferAddress(byte_buffer));
}

}  // namespace media

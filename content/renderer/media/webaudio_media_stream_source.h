// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_MEDIA_WEBAUDIO_MEDIA_STREAM_SOURCE_H_
#define CONTENT_RENDERER_MEDIA_WEBAUDIO_MEDIA_STREAM_SOURCE_H_

#include <memory>

#include "base/time/time.h"
#include "content/renderer/media/media_stream_audio_source.h"
#include "media/base/audio_bus.h"
#include "media/base/audio_push_fifo.h"
#include "third_party/WebKit/public/platform/WebAudioDestinationConsumer.h"
#include "third_party/WebKit/public/platform/WebMediaStreamSource.h"
#include "third_party/WebKit/public/platform/WebVector.h"

namespace content {

// Implements the WebAudioDestinationConsumer interface to provide a source of
// audio data (i.e., the output from a graph of WebAudio nodes) to one or more
// MediaStreamAudioTracks. Audio data is transported directly to the tracks in
// 10 ms chunks.
class WebAudioMediaStreamSource final
    : public MediaStreamAudioSource,
      public blink::WebAudioDestinationConsumer {
 public:
  explicit WebAudioMediaStreamSource(blink::WebMediaStreamSource* blink_source);

  ~WebAudioMediaStreamSource() override;

 private:
  // WebAudioDestinationConsumer implementation.
  //
  // Note: Blink ensures setFormat() and consumeAudio() are not called
  // concurrently across threads, but these methods could be called on any
  // thread.
  void SetFormat(size_t number_of_channels, float sample_rate) override;
  void ConsumeAudio(const blink::WebVector<const float*>& audio_data,
                    size_t number_of_frames) override;

  // Called by AudioPushFifo zero or more times during the call to
  // consumeAudio().  Delivers audio data with the required buffer size to the
  // tracks.
  void DeliverRebufferedAudio(const media::AudioBus& audio_bus,
                              int frame_delay);

  // MediaStreamAudioSource implementation.
  bool EnsureSourceIsStarted() final;
  void EnsureSourceIsStopped() final;

  // In debug builds, check that all methods that could cause object graph
  // or data flow changes are being called on the main thread.
  base::ThreadChecker thread_checker_;

  // True while this WebAudioMediaStreamSource is registered with
  // |blink_source_| and is consuming audio.
  bool is_registered_consumer_;

  // A wrapper used for providing audio to |fifo_|.
  std::unique_ptr<media::AudioBus> wrapper_bus_;

  // Takes in the audio data passed to consumeAudio() and re-buffers it into 10
  // ms chunks for the tracks. This ensures each chunk of audio delivered to
  // the tracks has the required buffer size, regardless of the amount of audio
  // provided via each consumeAudio() call.
  media::AudioPushFifo fifo_;

  // Used to pass the reference timestamp between DeliverDecodedAudio() and
  // DeliverRebufferedAudio().
  base::TimeTicks current_reference_time_;

  // This object registers with a blink::WebMediaStreamSource. We keep track of
  // that in order to be able to deregister before stopping this source.
  blink::WebMediaStreamSource blink_source_;

  DISALLOW_COPY_AND_ASSIGN(WebAudioMediaStreamSource);
};

}  // namespace content

#endif  // CONTENT_RENDERER_MEDIA_WEBAUDIO_MEDIA_STREAM_SOURCE_H_

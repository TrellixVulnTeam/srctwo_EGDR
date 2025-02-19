// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_MEDIA_MEDIA_STREAM_AUDIO_LEVEL_CALCULATOR_H_
#define CONTENT_RENDERER_MEDIA_MEDIA_STREAM_AUDIO_LEVEL_CALCULATOR_H_

#include "base/memory/ref_counted.h"
#include "base/synchronization/lock.h"
#include "content/common/content_export.h"

namespace media {
class AudioBus;
}

namespace content {

// This class is used by the WebRtcAudioCapturer to calculate the level of the
// audio signal. And the audio level will be eventually used by the volume
// animation UI.
//
// The algorithm used by this class is the same as how it is done in
// third_party/webrtc/voice_engine/level_indicator.cc.
class CONTENT_EXPORT MediaStreamAudioLevelCalculator {
 public:
  // Provides thread-safe access to the current signal level.  This object is
  // intended to be passed to modules running on other threads that poll for the
  // current signal level.
  class CONTENT_EXPORT Level : public base::RefCountedThreadSafe<Level> {
   public:
    float GetCurrent() const;

   private:
    friend class MediaStreamAudioLevelCalculator;
    friend class base::RefCountedThreadSafe<Level>;

    Level();
    ~Level();

    void Set(float level);

    mutable base::Lock lock_;
    float level_;
  };

  MediaStreamAudioLevelCalculator();
  ~MediaStreamAudioLevelCalculator();

  const scoped_refptr<Level>& level() const { return level_; }

  // Scans the audio signal in |audio_bus| and computes a new signal level
  // exposed by Level.  If |assume_nonzero_energy| is true, then a completely
  // zero'ed-out |audio_bus| will be accounted for as having a very faint,
  // non-zero level.
  void Calculate(const media::AudioBus& audio_bus, bool assume_nonzero_energy);

 private:
  int counter_;
  float max_amplitude_;
  const scoped_refptr<Level> level_;
};

}  // namespace content

#endif  // CONTENT_RENDERER_MEDIA_MEDIA_STREAM_AUDIO_LEVEL_CALCULATOR_H_

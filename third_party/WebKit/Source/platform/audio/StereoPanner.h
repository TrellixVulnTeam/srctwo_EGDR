// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef StereoPanner_h
#define StereoPanner_h

#include "platform/PlatformExport.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/Noncopyable.h"

namespace blink {

class AudioBus;

// Implement the equal-power panning algorithm for mono or stereo input. See:
// https://webaudio.github.io/web-audio-api/#Spatialzation-equal-power-panning

class PLATFORM_EXPORT StereoPanner {
  USING_FAST_MALLOC(StereoPanner);
  WTF_MAKE_NONCOPYABLE(StereoPanner);

 public:
  static std::unique_ptr<StereoPanner> Create(float sample_rate);
  ~StereoPanner(){};

  void PanWithSampleAccurateValues(const AudioBus* input_bus,
                                   AudioBus* output_bus,
                                   const float* pan_values,
                                   size_t frames_to_process);
  void PanToTargetValue(const AudioBus* input_bus,
                        AudioBus* output_bus,
                        float pan_value,
                        size_t frames_to_process);

 private:
  explicit StereoPanner(float sample_rate);

  bool is_first_render_;
  double smoothing_constant_;

  double pan_;
};

}  // namespace blink

#endif  // StereoPanner_h

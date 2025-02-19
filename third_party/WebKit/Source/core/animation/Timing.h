/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef Timing_h
#define Timing_h

#include "core/style/DataEquivalency.h"
#include "platform/animation/CompositorAnimation.h"
#include "platform/animation/TimingFunction.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/MathExtras.h"
#include "platform/wtf/RefPtr.h"

namespace blink {

struct Timing {
  USING_FAST_MALLOC(Timing);

 public:
  using FillMode = CompositorAnimation::FillMode;
  using PlaybackDirection = CompositorAnimation::Direction;

  static String FillModeString(FillMode);
  static String PlaybackDirectionString(PlaybackDirection);

  static const Timing& Defaults() {
    DEFINE_STATIC_LOCAL(Timing, timing, ());
    return timing;
  }

  Timing()
      : start_delay(0),
        end_delay(0),
        fill_mode(FillMode::AUTO),
        iteration_start(0),
        iteration_count(1),
        iteration_duration(std::numeric_limits<double>::quiet_NaN()),
        playback_rate(1),
        direction(PlaybackDirection::NORMAL),
        timing_function(LinearTimingFunction::Shared()) {}

  void AssertValid() const {
    DCHECK(std::isfinite(start_delay));
    DCHECK(std::isfinite(end_delay));
    DCHECK(std::isfinite(iteration_start));
    DCHECK_GE(iteration_start, 0);
    DCHECK_GE(iteration_count, 0);
    DCHECK(std::isnan(iteration_duration) || iteration_duration >= 0);
    DCHECK(std::isfinite(playback_rate));
    DCHECK(timing_function);
  }

  bool operator==(const Timing& other) const {
    return start_delay == other.start_delay && end_delay == other.end_delay &&
           fill_mode == other.fill_mode &&
           iteration_start == other.iteration_start &&
           iteration_count == other.iteration_count &&
           ((std::isnan(iteration_duration) &&
             std::isnan(other.iteration_duration)) ||
            iteration_duration == other.iteration_duration) &&
           playback_rate == other.playback_rate &&
           direction == other.direction &&
           DataEquivalent(timing_function.Get(), other.timing_function.Get());
  }

  bool operator!=(const Timing& other) const { return !(*this == other); }

  double start_delay;
  double end_delay;
  FillMode fill_mode;
  double iteration_start;
  double iteration_count;
  double iteration_duration;

  // TODO(crbug.com/630915) Remove playbackRate
  double playback_rate;
  PlaybackDirection direction;
  RefPtr<TimingFunction> timing_function;
};

}  // namespace blink

#endif

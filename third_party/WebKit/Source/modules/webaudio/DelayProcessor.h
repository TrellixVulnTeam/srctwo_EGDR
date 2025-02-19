/*
 * Copyright (C) 2010, Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#ifndef DelayProcessor_h
#define DelayProcessor_h

#include <memory>
#include "modules/webaudio/AudioParam.h"
#include "platform/audio/AudioDSPKernelProcessor.h"
#include "platform/wtf/RefPtr.h"

namespace blink {

class AudioDSPKernel;

class DelayProcessor final : public AudioDSPKernelProcessor {
 public:
  DelayProcessor(float sample_rate,
                 unsigned number_of_channels,
                 AudioParamHandler& delay_time,
                 double max_delay_time);
  ~DelayProcessor() override;

  std::unique_ptr<AudioDSPKernel> CreateKernel() override;

  void ProcessOnlyAudioParams(size_t frames_to_process) override;

  AudioParamHandler& DelayTime() const { return *delay_time_; }
  double MaxDelayTime() { return max_delay_time_; }

 private:
  RefPtr<AudioParamHandler> delay_time_;
  double max_delay_time_;
};

}  // namespace blink

#endif  // DelayProcessor_h

/*
 * Copyright (C) 2011, Google Inc. All rights reserved.
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

#ifndef BiquadFilterNode_h
#define BiquadFilterNode_h

#include "core/typed_arrays/ArrayBufferViewHelpers.h"
#include "core/typed_arrays/DOMTypedArray.h"
#include "modules/webaudio/AudioNode.h"
#include "modules/webaudio/BiquadProcessor.h"

namespace blink {

class BaseAudioContext;
class AudioParam;
class BiquadFilterOptions;

class BiquadFilterNode final : public AudioNode {
  DEFINE_WRAPPERTYPEINFO();

 public:
  // These must be defined as in the .idl file and must match those in the
  // BiquadProcessor class.
  enum {
    LOWPASS = 0,
    HIGHPASS = 1,
    BANDPASS = 2,
    LOWSHELF = 3,
    HIGHSHELF = 4,
    PEAKING = 5,
    NOTCH = 6,
    ALLPASS = 7
  };

  static BiquadFilterNode* Create(BaseAudioContext&, ExceptionState&);
  static BiquadFilterNode* Create(BaseAudioContext*,
                                  const BiquadFilterOptions&,
                                  ExceptionState&);

  DECLARE_VIRTUAL_TRACE();

  String type() const;
  void setType(const String&);

  AudioParam* frequency() { return frequency_; }
  AudioParam* q() { return q_; }
  AudioParam* gain() { return gain_; }
  AudioParam* detune() { return detune_; }

  // Get the magnitude and phase response of the filter at the given
  // set of frequencies (in Hz). The phase response is in radians.
  void getFrequencyResponse(NotShared<const DOMFloat32Array> frequency_hz,
                            NotShared<DOMFloat32Array> mag_response,
                            NotShared<DOMFloat32Array> phase_response);

 private:
  BiquadFilterNode(BaseAudioContext&);

  BiquadProcessor* GetBiquadProcessor() const;
  bool setType(unsigned);  // Returns true on success.

  Member<AudioParam> frequency_;
  Member<AudioParam> q_;
  Member<AudioParam> gain_;
  Member<AudioParam> detune_;
};

}  // namespace blink

#endif  // BiquadFilterNode_h

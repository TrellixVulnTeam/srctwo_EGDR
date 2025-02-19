/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef AudioBus_h
#define AudioBus_h

#include <memory>
#include "platform/audio/AudioChannel.h"
#include "platform/wtf/Noncopyable.h"
#include "platform/wtf/ThreadSafeRefCounted.h"
#include "platform/wtf/Vector.h"

namespace blink {

// An AudioBus represents a collection of one or more AudioChannels.
// The data layout is "planar" as opposed to "interleaved".  An AudioBus with
// one channel is mono, an AudioBus with two channels is stereo, etc.
class PLATFORM_EXPORT AudioBus : public ThreadSafeRefCounted<AudioBus> {
  WTF_MAKE_NONCOPYABLE(AudioBus);

 public:
  enum {
    kChannelLeft = 0,
    kChannelRight = 1,
    kChannelCenter = 2,  // center and mono are the same
    kChannelMono = 2,
    kChannelLFE = 3,
    kChannelSurroundLeft = 4,
    kChannelSurroundRight = 5,
  };

  enum {
    kLayoutCanonical = 0
    // Can define non-standard layouts here
  };

  enum ChannelInterpretation {
    kSpeakers,
    kDiscrete,
  };

  // allocate indicates whether or not to initially have the AudioChannels
  // created with managed storage.  Normal usage is to pass true here, in which
  // case the AudioChannels will memory-manage their own storage.  If allocate
  // is false then setChannelMemory() has to be called later on for each
  // channel before the AudioBus is useable...
  static PassRefPtr<AudioBus> Create(unsigned number_of_channels,
                                     size_t length,
                                     bool allocate = true);

  // Tells the given channel to use an externally allocated buffer.
  void SetChannelMemory(unsigned channel_index, float* storage, size_t length);

  // Channels
  unsigned NumberOfChannels() const { return channels_.size(); }

  AudioChannel* Channel(unsigned channel) { return channels_[channel].get(); }
  const AudioChannel* Channel(unsigned channel) const {
    return const_cast<AudioBus*>(this)->channels_[channel].get();
  }
  AudioChannel* ChannelByType(unsigned type);
  const AudioChannel* ChannelByType(unsigned type) const;

  // Number of sample-frames
  size_t length() const { return length_; }

  // resizeSmaller() can only be called with a new length <= the current length.
  // The data stored in the bus will remain undisturbed.
  void ResizeSmaller(size_t new_length);

  // Sample-rate : 0.0 if unknown or "don't care"
  float SampleRate() const { return sample_rate_; }
  void SetSampleRate(float sample_rate) { sample_rate_ = sample_rate; }

  // Zeroes all channels.
  void Zero();

  // Clears the silent flag on all channels.
  void ClearSilentFlag();

  // Returns true if the silent bit is set on all channels.
  bool IsSilent() const;

  // Returns true if the channel count and frame-size match.
  bool TopologyMatches(const AudioBus& source_bus) const;

  // Creates a new buffer from a range in the source buffer.
  // 0 may be returned if the range does not fit in the sourceBuffer
  static PassRefPtr<AudioBus> CreateBufferFromRange(
      const AudioBus* source_buffer,
      unsigned start_frame,
      unsigned end_frame);

  // Creates a new AudioBus by sample-rate converting sourceBus to the
  // newSampleRate.
  // setSampleRate() must have been previously called on sourceBus.
  // Note: sample-rate conversion is already handled in the file-reading code
  // for the mac port, so we don't need this.
  static PassRefPtr<AudioBus> CreateBySampleRateConverting(
      const AudioBus* source_bus,
      bool mix_to_mono,
      double new_sample_rate);

  // Creates a new AudioBus by mixing all the channels down to mono.
  // If sourceBus is already mono, then the returned AudioBus will simply be a
  // copy.
  static PassRefPtr<AudioBus> CreateByMixingToMono(const AudioBus* source_bus);

  // Scales all samples by the same amount.
  void Scale(float scale);

  void Reset() { is_first_time_ = true; }  // for de-zippering

  // Copies the samples from the source bus to this one.
  // This is just a simple per-channel copy if the number of channels match,
  // otherwise an up-mix or down-mix is done.
  void CopyFrom(const AudioBus& source_bus, ChannelInterpretation = kSpeakers);

  // Sums the samples from the source bus to this one.
  // This is just a simple per-channel summing if the number of channels match,
  // otherwise an up-mix or down-mix is done.
  void SumFrom(const AudioBus& source_bus, ChannelInterpretation = kSpeakers);

  // Copy each channel from sourceBus into our corresponding channel.
  // We scale by targetGain (and our own internal gain m_busGain), performing
  // "de-zippering" to smoothly change from *lastMixGain to
  // (targetGain*m_busGain).  The caller is responsible for setting up
  // lastMixGain to point to storage which is unique for every "stream" which
  // will be applied to this bus.
  // This represents the dezippering memory.
  void CopyWithGainFrom(const AudioBus& source_bus,
                        float* last_mix_gain,
                        float target_gain);

  // Copies the sourceBus by scaling with sample-accurate gain values.
  void CopyWithSampleAccurateGainValuesFrom(const AudioBus& source_bus,
                                            float* gain_values,
                                            unsigned number_of_gain_values);

  // Returns maximum absolute value across all channels (useful for
  // normalization).
  float MaxAbsValue() const;

  // Makes maximum absolute value == 1.0 (if possible).
  void Normalize();

  static PassRefPtr<AudioBus> GetDataResource(const char* name,
                                              float sample_rate);

 protected:
  AudioBus() {}

  AudioBus(unsigned number_of_channels, size_t length, bool allocate);

  void DiscreteSumFrom(const AudioBus&);

  // Up/down-mix by in-place summing upon the existing channel content.
  // http://webaudio.github.io/web-audio-api/#channel-up-mixing-and-down-mixing
  void SumFromByUpMixing(const AudioBus&);
  void SumFromByDownMixing(const AudioBus&);

  size_t length_;
  Vector<std::unique_ptr<AudioChannel>> channels_;
  int layout_;
  float bus_gain_;
  std::unique_ptr<AudioFloatArray> dezipper_gain_values_;
  bool is_first_time_;
  float sample_rate_;  // 0.0 if unknown or N/A
};

}  // namespace blink

#endif  // AudioBus_h

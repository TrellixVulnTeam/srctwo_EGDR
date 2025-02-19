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

#include "modules/webaudio/RealtimeAnalyser.h"
#include <limits.h>
#include <algorithm>
#include <complex>
#include "platform/audio/AudioBus.h"
#include "platform/audio/AudioUtilities.h"
#include "platform/audio/VectorMath.h"
#include "platform/wtf/MathExtras.h"
#include "platform/wtf/PtrUtil.h"

namespace blink {

const double RealtimeAnalyser::kDefaultSmoothingTimeConstant = 0.8;
const double RealtimeAnalyser::kDefaultMinDecibels = -100;
const double RealtimeAnalyser::kDefaultMaxDecibels = -30;

const unsigned RealtimeAnalyser::kDefaultFFTSize = 2048;
// All FFT implementations are expected to handle power-of-two sizes
// MinFFTSize <= size <= MaxFFTSize.
const unsigned RealtimeAnalyser::kMinFFTSize = 32;
const unsigned RealtimeAnalyser::kMaxFFTSize = 32768;
const unsigned RealtimeAnalyser::kInputBufferSize =
    RealtimeAnalyser::kMaxFFTSize * 2;

RealtimeAnalyser::RealtimeAnalyser()
    : input_buffer_(kInputBufferSize),
      write_index_(0),
      down_mix_bus_(AudioBus::Create(1, AudioUtilities::kRenderQuantumFrames)),
      fft_size_(kDefaultFFTSize),
      magnitude_buffer_(kDefaultFFTSize / 2),
      smoothing_time_constant_(kDefaultSmoothingTimeConstant),
      min_decibels_(kDefaultMinDecibels),
      max_decibels_(kDefaultMaxDecibels),
      last_analysis_time_(-1) {
  analysis_frame_ = WTF::MakeUnique<FFTFrame>(kDefaultFFTSize);
}

bool RealtimeAnalyser::SetFftSize(size_t size) {
  DCHECK(IsMainThread());

  // Only allow powers of two within the allowed range.
  if (size > kMaxFFTSize || size < kMinFFTSize ||
      !AudioUtilities::IsPowerOfTwo(size))
    return false;

  if (fft_size_ != size) {
    analysis_frame_ = WTF::MakeUnique<FFTFrame>(size);
    // m_magnitudeBuffer has size = fftSize / 2 because it contains floats
    // reduced from complex values in m_analysisFrame.
    magnitude_buffer_.Allocate(size / 2);
    fft_size_ = size;
  }

  return true;
}

void RealtimeAnalyser::WriteInput(AudioBus* bus, size_t frames_to_process) {
  bool is_bus_good = bus && bus->NumberOfChannels() > 0 &&
                     bus->Channel(0)->length() >= frames_to_process;
  DCHECK(is_bus_good);
  if (!is_bus_good)
    return;

  // FIXME : allow to work with non-FFTSize divisible chunking
  bool is_destination_good =
      write_index_ < input_buffer_.size() &&
      write_index_ + frames_to_process <= input_buffer_.size();
  DCHECK(is_destination_good);
  if (!is_destination_good)
    return;

  // Perform real-time analysis
  float* dest = input_buffer_.Data() + write_index_;

  // Clear the bus and downmix the input according to the down mixing rules.
  // Then save the result in the m_inputBuffer at the appropriate place.
  down_mix_bus_->Zero();
  down_mix_bus_->SumFrom(*bus);
  memcpy(dest, down_mix_bus_->Channel(0)->Data(),
         frames_to_process * sizeof(*dest));

  write_index_ += frames_to_process;
  if (write_index_ >= kInputBufferSize)
    write_index_ = 0;
}

namespace {

void ApplyWindow(float* p, size_t n) {
  DCHECK(IsMainThread());

  // Blackman window
  double alpha = 0.16;
  double a0 = 0.5 * (1 - alpha);
  double a1 = 0.5;
  double a2 = 0.5 * alpha;

  for (unsigned i = 0; i < n; ++i) {
    double x = static_cast<double>(i) / static_cast<double>(n);
    double window =
        a0 - a1 * cos(twoPiDouble * x) + a2 * cos(twoPiDouble * 2.0 * x);
    p[i] *= float(window);
  }
}

}  // namespace

void RealtimeAnalyser::DoFFTAnalysis() {
  DCHECK(IsMainThread());

  // Unroll the input buffer into a temporary buffer, where we'll apply an
  // analysis window followed by an FFT.
  size_t fft_size = this->FftSize();

  AudioFloatArray temporary_buffer(fft_size);
  float* input_buffer = input_buffer_.Data();
  float* temp_p = temporary_buffer.Data();

  // Take the previous fftSize values from the input buffer and copy into the
  // temporary buffer.
  unsigned write_index = write_index_;
  if (write_index < fft_size) {
    memcpy(temp_p, input_buffer + write_index - fft_size + kInputBufferSize,
           sizeof(*temp_p) * (fft_size - write_index));
    memcpy(temp_p + fft_size - write_index, input_buffer,
           sizeof(*temp_p) * write_index);
  } else {
    memcpy(temp_p, input_buffer + write_index - fft_size,
           sizeof(*temp_p) * fft_size);
  }

  // Window the input samples.
  ApplyWindow(temp_p, fft_size);

  // Do the analysis.
  analysis_frame_->DoFFT(temp_p);

  float* real_p = analysis_frame_->RealData();
  float* imag_p = analysis_frame_->ImagData();

  // Blow away the packed nyquist component.
  imag_p[0] = 0;

  // Normalize so than an input sine wave at 0dBfs registers as 0dBfs (undo FFT
  // scaling factor).
  const double magnitude_scale = 1.0 / fft_size;

  // A value of 0 does no averaging with the previous result.  Larger values
  // produce slower, but smoother changes.
  double k = smoothing_time_constant_;
  k = std::max(0.0, k);
  k = std::min(1.0, k);

  // Convert the analysis data from complex to magnitude and average with the
  // previous result.
  float* destination = MagnitudeBuffer().Data();
  size_t n = MagnitudeBuffer().size();
  for (size_t i = 0; i < n; ++i) {
    std::complex<double> c(real_p[i], imag_p[i]);
    double scalar_magnitude = abs(c) * magnitude_scale;
    destination[i] = float(k * destination[i] + (1 - k) * scalar_magnitude);
  }
}

void RealtimeAnalyser::ConvertFloatToDb(DOMFloat32Array* destination_array) {
  // Convert from linear magnitude to floating-point decibels.
  unsigned source_length = MagnitudeBuffer().size();
  size_t len = std::min(source_length, destination_array->length());
  if (len > 0) {
    const float* source = MagnitudeBuffer().Data();
    float* destination = destination_array->Data();

    for (unsigned i = 0; i < len; ++i) {
      float linear_value = source[i];
      double db_mag = AudioUtilities::LinearToDecibels(linear_value);
      destination[i] = float(db_mag);
    }
  }
}

void RealtimeAnalyser::GetFloatFrequencyData(DOMFloat32Array* destination_array,
                                             double current_time) {
  DCHECK(IsMainThread());
  DCHECK(destination_array);

  if (current_time <= last_analysis_time_) {
    ConvertFloatToDb(destination_array);
    return;
  }

  // Time has advanced since the last call; update the FFT data.
  last_analysis_time_ = current_time;
  DoFFTAnalysis();

  ConvertFloatToDb(destination_array);
}

void RealtimeAnalyser::ConvertToByteData(DOMUint8Array* destination_array) {
  // Convert from linear magnitude to unsigned-byte decibels.
  unsigned source_length = MagnitudeBuffer().size();
  size_t len = std::min(source_length, destination_array->length());
  if (len > 0) {
    const double range_scale_factor = max_decibels_ == min_decibels_
                                          ? 1
                                          : 1 / (max_decibels_ - min_decibels_);
    const double min_decibels = min_decibels_;

    const float* source = MagnitudeBuffer().Data();
    unsigned char* destination = destination_array->Data();

    for (unsigned i = 0; i < len; ++i) {
      float linear_value = source[i];
      double db_mag = AudioUtilities::LinearToDecibels(linear_value);

      // The range m_minDecibels to m_maxDecibels will be scaled to byte values
      // from 0 to UCHAR_MAX.
      double scaled_value =
          UCHAR_MAX * (db_mag - min_decibels) * range_scale_factor;

      // Clip to valid range.
      if (scaled_value < 0)
        scaled_value = 0;
      if (scaled_value > UCHAR_MAX)
        scaled_value = UCHAR_MAX;

      destination[i] = static_cast<unsigned char>(scaled_value);
    }
  }
}

void RealtimeAnalyser::GetByteFrequencyData(DOMUint8Array* destination_array,
                                            double current_time) {
  DCHECK(IsMainThread());
  DCHECK(destination_array);

  if (current_time <= last_analysis_time_) {
    // FIXME: Is it worth caching the data so we don't have to do the conversion
    // every time?  Perhaps not, since we expect many calls in the same
    // rendering quantum.
    ConvertToByteData(destination_array);
    return;
  }

  // Time has advanced since the last call; update the FFT data.
  last_analysis_time_ = current_time;
  DoFFTAnalysis();

  ConvertToByteData(destination_array);
}

void RealtimeAnalyser::GetFloatTimeDomainData(
    DOMFloat32Array* destination_array) {
  DCHECK(IsMainThread());
  DCHECK(destination_array);

  unsigned fft_size = this->FftSize();
  size_t len = std::min(fft_size, destination_array->length());
  if (len > 0) {
    bool is_input_buffer_good = input_buffer_.size() == kInputBufferSize &&
                                input_buffer_.size() > fft_size;
    DCHECK(is_input_buffer_good);
    if (!is_input_buffer_good)
      return;

    float* input_buffer = input_buffer_.Data();
    float* destination = destination_array->Data();

    unsigned write_index = write_index_;

    for (unsigned i = 0; i < len; ++i) {
      // Buffer access is protected due to modulo operation.
      float value =
          input_buffer[(i + write_index - fft_size + kInputBufferSize) %
                       kInputBufferSize];

      destination[i] = value;
    }
  }
}

void RealtimeAnalyser::GetByteTimeDomainData(DOMUint8Array* destination_array) {
  DCHECK(IsMainThread());
  DCHECK(destination_array);

  unsigned fft_size = this->FftSize();
  size_t len = std::min(fft_size, destination_array->length());
  if (len > 0) {
    bool is_input_buffer_good = input_buffer_.size() == kInputBufferSize &&
                                input_buffer_.size() > fft_size;
    DCHECK(is_input_buffer_good);
    if (!is_input_buffer_good)
      return;

    float* input_buffer = input_buffer_.Data();
    unsigned char* destination = destination_array->Data();

    unsigned write_index = write_index_;

    for (unsigned i = 0; i < len; ++i) {
      // Buffer access is protected due to modulo operation.
      float value =
          input_buffer[(i + write_index - fft_size + kInputBufferSize) %
                       kInputBufferSize];

      // Scale from nominal -1 -> +1 to unsigned byte.
      double scaled_value = 128 * (value + 1);

      // Clip to valid range.
      if (scaled_value < 0)
        scaled_value = 0;
      if (scaled_value > UCHAR_MAX)
        scaled_value = UCHAR_MAX;

      destination[i] = static_cast<unsigned char>(scaled_value);
    }
  }
}

}  // namespace blink

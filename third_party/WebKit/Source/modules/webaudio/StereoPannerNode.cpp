// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/webaudio/StereoPannerNode.h"
#include "bindings/core/v8/ExceptionMessages.h"
#include "bindings/core/v8/ExceptionState.h"
#include "core/dom/ExceptionCode.h"
#include "core/dom/ExecutionContext.h"
#include "modules/webaudio/AudioNodeInput.h"
#include "modules/webaudio/AudioNodeOutput.h"
#include "modules/webaudio/BaseAudioContext.h"
#include "modules/webaudio/StereoPannerOptions.h"
#include "platform/audio/StereoPanner.h"
#include "platform/wtf/MathExtras.h"

namespace blink {

StereoPannerHandler::StereoPannerHandler(AudioNode& node,
                                         float sample_rate,
                                         AudioParamHandler& pan)
    : AudioHandler(kNodeTypeStereoPanner, node, sample_rate),
      pan_(pan),
      sample_accurate_pan_values_(AudioUtilities::kRenderQuantumFrames) {
  AddInput();
  AddOutput(2);

  // The node-specific default mixing rules declare that StereoPannerNode
  // can handle mono to stereo and stereo to stereo conversion.
  channel_count_ = 2;
  SetInternalChannelCountMode(kClampedMax);
  SetInternalChannelInterpretation(AudioBus::kSpeakers);

  Initialize();
}

PassRefPtr<StereoPannerHandler> StereoPannerHandler::Create(
    AudioNode& node,
    float sample_rate,
    AudioParamHandler& pan) {
  return AdoptRef(new StereoPannerHandler(node, sample_rate, pan));
}

StereoPannerHandler::~StereoPannerHandler() {
  Uninitialize();
}

void StereoPannerHandler::Process(size_t frames_to_process) {
  AudioBus* output_bus = Output(0).Bus();

  if (!IsInitialized() || !Input(0).IsConnected() || !stereo_panner_.get()) {
    output_bus->Zero();
    return;
  }

  AudioBus* input_bus = Input(0).Bus();
  if (!input_bus) {
    output_bus->Zero();
    return;
  }

  if (pan_->HasSampleAccurateValues()) {
    // Apply sample-accurate panning specified by AudioParam automation.
    DCHECK_LE(frames_to_process, sample_accurate_pan_values_.size());
    if (frames_to_process <= sample_accurate_pan_values_.size()) {
      float* pan_values = sample_accurate_pan_values_.Data();
      pan_->CalculateSampleAccurateValues(pan_values, frames_to_process);
      stereo_panner_->PanWithSampleAccurateValues(
          input_bus, output_bus, pan_values, frames_to_process);
    }
  } else {
    stereo_panner_->PanToTargetValue(input_bus, output_bus, pan_->Value(),
                                     frames_to_process);
  }
}

void StereoPannerHandler::ProcessOnlyAudioParams(size_t frames_to_process) {
  float values[AudioUtilities::kRenderQuantumFrames];
  DCHECK_LE(frames_to_process, AudioUtilities::kRenderQuantumFrames);

  pan_->CalculateSampleAccurateValues(values, frames_to_process);
}

void StereoPannerHandler::Initialize() {
  if (IsInitialized())
    return;

  stereo_panner_ = StereoPanner::Create(Context()->sampleRate());

  AudioHandler::Initialize();
}

void StereoPannerHandler::SetChannelCount(unsigned long channel_count,
                                          ExceptionState& exception_state) {
  DCHECK(IsMainThread());
  BaseAudioContext::AutoLocker locker(Context());

  // A PannerNode only supports 1 or 2 channels
  if (channel_count > 0 && channel_count <= 2) {
    if (channel_count_ != channel_count) {
      channel_count_ = channel_count;
      if (InternalChannelCountMode() != kMax)
        UpdateChannelsForInputs();
    }
  } else {
    exception_state.ThrowDOMException(
        kNotSupportedError, ExceptionMessages::IndexOutsideRange<unsigned long>(
                                "channelCount", channel_count, 1,
                                ExceptionMessages::kInclusiveBound, 2,
                                ExceptionMessages::kInclusiveBound));
  }
}

void StereoPannerHandler::SetChannelCountMode(const String& mode,
                                              ExceptionState& exception_state) {
  DCHECK(IsMainThread());
  BaseAudioContext::AutoLocker locker(Context());

  ChannelCountMode old_mode = InternalChannelCountMode();

  if (mode == "clamped-max") {
    new_channel_count_mode_ = kClampedMax;
  } else if (mode == "explicit") {
    new_channel_count_mode_ = kExplicit;
  } else if (mode == "max") {
    // This is not supported for a StereoPannerNode, which can only handle
    // 1 or 2 channels.
    exception_state.ThrowDOMException(kNotSupportedError,
                                      "StereoPanner: 'max' is not allowed");
    new_channel_count_mode_ = old_mode;
  } else {
    // Do nothing for other invalid values.
    new_channel_count_mode_ = old_mode;
  }

  if (new_channel_count_mode_ != old_mode)
    Context()->GetDeferredTaskHandler().AddChangedChannelCountMode(this);
}

// ----------------------------------------------------------------

StereoPannerNode::StereoPannerNode(BaseAudioContext& context)
    : AudioNode(context),
      pan_(AudioParam::Create(context, kParamTypeStereoPannerPan, 0, -1, 1)) {
  SetHandler(StereoPannerHandler::Create(*this, context.sampleRate(),
                                         pan_->Handler()));
}

StereoPannerNode* StereoPannerNode::Create(BaseAudioContext& context,
                                           ExceptionState& exception_state) {
  DCHECK(IsMainThread());

  if (context.IsContextClosed()) {
    context.ThrowExceptionForClosedState(exception_state);
    return nullptr;
  }

  return new StereoPannerNode(context);
}

StereoPannerNode* StereoPannerNode::Create(BaseAudioContext* context,
                                           const StereoPannerOptions& options,
                                           ExceptionState& exception_state) {
  StereoPannerNode* node = Create(*context, exception_state);

  if (!node)
    return nullptr;

  node->HandleChannelOptions(options, exception_state);

  node->pan()->setValue(options.pan());

  return node;
}

DEFINE_TRACE(StereoPannerNode) {
  visitor->Trace(pan_);
  AudioNode::Trace(visitor);
}

AudioParam* StereoPannerNode::pan() const {
  return pan_;
}

}  // namespace blink

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

#include "modules/webaudio/WaveShaperNode.h"
#include "bindings/core/v8/ExceptionMessages.h"
#include "bindings/core/v8/ExceptionState.h"
#include "core/dom/ExceptionCode.h"
#include "modules/webaudio/AudioBasicProcessorHandler.h"
#include "modules/webaudio/BaseAudioContext.h"
#include "modules/webaudio/WaveShaperOptions.h"
#include "platform/wtf/PtrUtil.h"

namespace blink {

WaveShaperNode::WaveShaperNode(BaseAudioContext& context) : AudioNode(context) {
  SetHandler(AudioBasicProcessorHandler::Create(
      AudioHandler::kNodeTypeWaveShaper, *this, context.sampleRate(),
      WTF::WrapUnique(new WaveShaperProcessor(context.sampleRate(), 1))));

  Handler().Initialize();
}

WaveShaperNode* WaveShaperNode::Create(BaseAudioContext& context,
                                       ExceptionState& exception_state) {
  DCHECK(IsMainThread());

  if (context.IsContextClosed()) {
    context.ThrowExceptionForClosedState(exception_state);
    return nullptr;
  }

  return new WaveShaperNode(context);
}

WaveShaperNode* WaveShaperNode::Create(BaseAudioContext* context,
                                       const WaveShaperOptions& options,
                                       ExceptionState& exception_state) {
  WaveShaperNode* node = Create(*context, exception_state);

  if (!node)
    return nullptr;

  node->HandleChannelOptions(options, exception_state);

  if (options.hasCurve())
    node->setCurve(options.curve(), exception_state);

  node->setOversample(options.oversample());

  return node;
}
WaveShaperProcessor* WaveShaperNode::GetWaveShaperProcessor() const {
  return static_cast<WaveShaperProcessor*>(
      static_cast<AudioBasicProcessorHandler&>(Handler()).Processor());
}

void WaveShaperNode::SetCurveImpl(const float* curve_data,
                                  unsigned curve_length,
                                  ExceptionState& exception_state) {
  DCHECK(IsMainThread());

  if (curve_data && curve_length < 2) {
    exception_state.ThrowDOMException(
        kInvalidAccessError,
        ExceptionMessages::IndexExceedsMinimumBound<unsigned>("curve length",
                                                              curve_length, 2));
    return;
  }

  GetWaveShaperProcessor()->SetCurve(curve_data, curve_length);
}

void WaveShaperNode::setCurve(NotShared<DOMFloat32Array> curve,
                              ExceptionState& exception_state) {
  DCHECK(IsMainThread());

  if (curve)
    SetCurveImpl(curve.View()->Data(), curve.View()->length(), exception_state);
  else
    SetCurveImpl(nullptr, 0, exception_state);
}

void WaveShaperNode::setCurve(const Vector<float>& curve,
                              ExceptionState& exception_state) {
  DCHECK(IsMainThread());

  SetCurveImpl(curve.data(), curve.size(), exception_state);
}

NotShared<DOMFloat32Array> WaveShaperNode::curve() {
  Vector<float>* curve = GetWaveShaperProcessor()->Curve();
  if (!curve)
    return NotShared<DOMFloat32Array>(nullptr);

  unsigned size = curve->size();
  RefPtr<WTF::Float32Array> new_curve = WTF::Float32Array::Create(size);

  memcpy(new_curve->Data(), curve->data(), sizeof(float) * size);

  return NotShared<DOMFloat32Array>(
      DOMFloat32Array::Create(std::move(new_curve)));
}

void WaveShaperNode::setOversample(const String& type) {
  DCHECK(IsMainThread());

  // This is to synchronize with the changes made in
  // AudioBasicProcessorNode::checkNumberOfChannelsForInput() where we can
  // initialize() and uninitialize().
  BaseAudioContext::AutoLocker context_locker(context());

  if (type == "none") {
    GetWaveShaperProcessor()->SetOversample(
        WaveShaperProcessor::kOverSampleNone);
  } else if (type == "2x") {
    GetWaveShaperProcessor()->SetOversample(WaveShaperProcessor::kOverSample2x);
  } else if (type == "4x") {
    GetWaveShaperProcessor()->SetOversample(WaveShaperProcessor::kOverSample4x);
  } else {
    NOTREACHED();
  }
}

String WaveShaperNode::oversample() const {
  switch (const_cast<WaveShaperNode*>(this)
              ->GetWaveShaperProcessor()
              ->Oversample()) {
    case WaveShaperProcessor::kOverSampleNone:
      return "none";
    case WaveShaperProcessor::kOverSample2x:
      return "2x";
    case WaveShaperProcessor::kOverSample4x:
      return "4x";
    default:
      NOTREACHED();
      return "none";
  }
}

}  // namespace blink

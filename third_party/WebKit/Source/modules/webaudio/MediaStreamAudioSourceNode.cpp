/*
 * Copyright (C) 2012, Google Inc. All rights reserved.
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

#include "modules/webaudio/MediaStreamAudioSourceNode.h"

#include <memory>
#include "bindings/core/v8/ExceptionState.h"
#include "core/dom/ExceptionCode.h"
#include "modules/webaudio/AudioNodeOutput.h"
#include "modules/webaudio/BaseAudioContext.h"
#include "modules/webaudio/MediaStreamAudioSourceOptions.h"
#include "platform/wtf/Locker.h"

namespace blink {

MediaStreamAudioSourceHandler::MediaStreamAudioSourceHandler(
    AudioNode& node,
    std::unique_ptr<AudioSourceProvider> audio_source_provider)
    : AudioHandler(kNodeTypeMediaStreamAudioSource,
                   node,
                   node.context()->sampleRate()),
      audio_source_provider_(std::move(audio_source_provider)),
      source_number_of_channels_(0) {
  // Default to stereo. This could change depending on the format of the
  // MediaStream's audio track.
  AddOutput(2);

  Initialize();
}

PassRefPtr<MediaStreamAudioSourceHandler> MediaStreamAudioSourceHandler::Create(
    AudioNode& node,
    std::unique_ptr<AudioSourceProvider> audio_source_provider) {
  return AdoptRef(new MediaStreamAudioSourceHandler(
      node, std::move(audio_source_provider)));
}

MediaStreamAudioSourceHandler::~MediaStreamAudioSourceHandler() {
  Uninitialize();
}

void MediaStreamAudioSourceHandler::SetFormat(size_t number_of_channels,
                                              float source_sample_rate) {
  if (number_of_channels != source_number_of_channels_ ||
      source_sample_rate != Context()->sampleRate()) {
    // The sample-rate must be equal to the context's sample-rate.
    if (!number_of_channels ||
        number_of_channels > BaseAudioContext::MaxNumberOfChannels() ||
        source_sample_rate != Context()->sampleRate()) {
      // process() will generate silence for these uninitialized values.
      DLOG(ERROR) << "setFormat(" << number_of_channels << ", "
                  << source_sample_rate << ") - unhandled format change";
      source_number_of_channels_ = 0;
      return;
    }

    // Synchronize with process().
    MutexLocker locker(process_lock_);

    source_number_of_channels_ = number_of_channels;

    {
      // The context must be locked when changing the number of output channels.
      BaseAudioContext::AutoLocker context_locker(Context());

      // Do any necesssary re-configuration to the output's number of channels.
      Output(0).SetNumberOfChannels(number_of_channels);
    }
  }
}

void MediaStreamAudioSourceHandler::Process(size_t number_of_frames) {
  AudioBus* output_bus = Output(0).Bus();

  if (!GetAudioSourceProvider()) {
    output_bus->Zero();
    return;
  }

  if (source_number_of_channels_ != output_bus->NumberOfChannels()) {
    output_bus->Zero();
    return;
  }

  // Use a tryLock() to avoid contention in the real-time audio thread.
  // If we fail to acquire the lock then the MediaStream must be in the middle
  // of a format change, so we output silence in this case.
  MutexTryLocker try_locker(process_lock_);
  if (try_locker.Locked()) {
    GetAudioSourceProvider()->ProvideInput(output_bus, number_of_frames);
  } else {
    // We failed to acquire the lock.
    output_bus->Zero();
  }
}

// ----------------------------------------------------------------

MediaStreamAudioSourceNode::MediaStreamAudioSourceNode(
    BaseAudioContext& context,
    MediaStream& media_stream,
    MediaStreamTrack* audio_track,
    std::unique_ptr<AudioSourceProvider> audio_source_provider)
    : AudioNode(context),
      audio_track_(audio_track),
      media_stream_(media_stream) {
  SetHandler(MediaStreamAudioSourceHandler::Create(
      *this, std::move(audio_source_provider)));
}

MediaStreamAudioSourceNode* MediaStreamAudioSourceNode::Create(
    BaseAudioContext& context,
    MediaStream& media_stream,
    ExceptionState& exception_state) {
  DCHECK(IsMainThread());

  if (context.IsContextClosed()) {
    context.ThrowExceptionForClosedState(exception_state);
    return nullptr;
  }

  MediaStreamTrackVector audio_tracks = media_stream.getAudioTracks();
  if (audio_tracks.IsEmpty()) {
    exception_state.ThrowDOMException(kInvalidStateError,
                                      "MediaStream has no audio track");
    return nullptr;
  }

  // Use the first audio track in the media stream.
  MediaStreamTrack* audio_track = audio_tracks[0];
  std::unique_ptr<AudioSourceProvider> provider =
      audio_track->CreateWebAudioSource();

  MediaStreamAudioSourceNode* node = new MediaStreamAudioSourceNode(
      context, media_stream, audio_track, std::move(provider));

  if (!node)
    return nullptr;

  // TODO(hongchan): Only stereo streams are supported right now. We should be
  // able to accept multi-channel streams.
  node->SetFormat(2, context.sampleRate());
  // context keeps reference until node is disconnected
  context.NotifySourceNodeStartedProcessing(node);

  return node;
}

MediaStreamAudioSourceNode* MediaStreamAudioSourceNode::Create(
    BaseAudioContext* context,
    const MediaStreamAudioSourceOptions& options,
    ExceptionState& exception_state) {
  return Create(*context, *options.mediaStream(), exception_state);
}

DEFINE_TRACE(MediaStreamAudioSourceNode) {
  visitor->Trace(audio_track_);
  visitor->Trace(media_stream_);
  AudioSourceProviderClient::Trace(visitor);
  AudioNode::Trace(visitor);
}

MediaStreamAudioSourceHandler&
MediaStreamAudioSourceNode::GetMediaStreamAudioSourceHandler() const {
  return static_cast<MediaStreamAudioSourceHandler&>(Handler());
}

MediaStream* MediaStreamAudioSourceNode::getMediaStream() const {
  return media_stream_;
}

void MediaStreamAudioSourceNode::SetFormat(size_t number_of_channels,
                                           float source_sample_rate) {
  GetMediaStreamAudioSourceHandler().SetFormat(number_of_channels,
                                               source_sample_rate);
}

}  // namespace blink

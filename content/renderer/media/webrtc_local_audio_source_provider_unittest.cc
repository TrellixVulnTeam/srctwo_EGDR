// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>

#include "base/logging.h"
#include "content/renderer/media/media_stream_audio_track.h"
#include "content/renderer/media/webrtc_local_audio_source_provider.h"
#include "media/base/audio_bus.h"
#include "media/base/audio_parameters.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/WebKit/public/platform/WebMediaStreamTrack.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "third_party/WebKit/public/web/WebHeap.h"

namespace content {

class WebRtcLocalAudioSourceProviderTest : public testing::Test {
 protected:
  void SetUp() override {
    source_params_.Reset(media::AudioParameters::AUDIO_PCM_LOW_LATENCY,
                         media::CHANNEL_LAYOUT_MONO, 48000, 16, 480);
    sink_params_.Reset(
        media::AudioParameters::AUDIO_PCM_LOW_LATENCY,
        media::CHANNEL_LAYOUT_STEREO, 44100, 16,
        WebRtcLocalAudioSourceProvider::kWebAudioRenderBufferSize);
    sink_bus_ = media::AudioBus::Create(sink_params_);
    blink::WebMediaStreamSource audio_source;
    audio_source.Initialize(blink::WebString::FromUTF8("dummy_source_id"),
                            blink::WebMediaStreamSource::kTypeAudio,
                            blink::WebString::FromUTF8("dummy_source_name"),
                            false /* remote */);
    blink_track_.Initialize(blink::WebString::FromUTF8("audio_track"),
                            audio_source);
    blink_track_.SetTrackData(new MediaStreamAudioTrack(true));
    source_provider_.reset(new WebRtcLocalAudioSourceProvider(blink_track_));
    source_provider_->SetSinkParamsForTesting(sink_params_);
    source_provider_->OnSetFormat(source_params_);
  }

  void TearDown() override {
    source_provider_.reset();
    blink_track_.Reset();
    blink::WebHeap::CollectAllGarbageForTesting();
  }

  media::AudioParameters source_params_;
  media::AudioParameters sink_params_;
  std::unique_ptr<media::AudioBus> sink_bus_;
  blink::WebMediaStreamTrack blink_track_;
  std::unique_ptr<WebRtcLocalAudioSourceProvider> source_provider_;
};

TEST_F(WebRtcLocalAudioSourceProviderTest, VerifyDataFlow) {
  // TODO(miu): This test should be re-worked so that the audio data and format
  // is feed into a MediaStreamAudioSource and, through the
  // MediaStreamAudioTrack, ultimately delivered to the |source_provider_|.

  // Point the WebVector into memory owned by |sink_bus_|.
  blink::WebVector<float*> audio_data(
      static_cast<size_t>(sink_bus_->channels()));
  for (size_t i = 0; i < audio_data.size(); ++i)
    audio_data[i] = sink_bus_->channel(i);

  // Enable the |source_provider_| by asking for data. This will inject
  // source_params_.frames_per_buffer() of zero into the resampler since there
  // no available data in the FIFO.
  source_provider_->ProvideInput(audio_data, sink_params_.frames_per_buffer());
  EXPECT_EQ(0, sink_bus_->channel(0)[0]);

  // Create a source AudioBus with channel data filled with non-zero values.
  const std::unique_ptr<media::AudioBus> source_bus =
      media::AudioBus::Create(source_params_);
  std::fill(source_bus->channel(0),
            source_bus->channel(0) + source_bus->frames(),
            0.5f);

  // Deliver data to |source_provider_|.
  base::TimeTicks estimated_capture_time = base::TimeTicks::Now();
  source_provider_->OnData(*source_bus, estimated_capture_time);

  // Consume the first packet in the resampler, which contains only zeros.
  // And the consumption of the data will trigger pulling the real packet from
  // the source provider FIFO into the resampler.
  // Note that we need to count in the provideInput() call a few lines above.
  for (int i = sink_params_.frames_per_buffer();
       i < source_params_.frames_per_buffer();
       i += sink_params_.frames_per_buffer()) {
    sink_bus_->Zero();
    source_provider_->ProvideInput(audio_data,
                                   sink_params_.frames_per_buffer());
    EXPECT_DOUBLE_EQ(0.0, sink_bus_->channel(0)[0]);
    EXPECT_DOUBLE_EQ(0.0, sink_bus_->channel(1)[0]);
  }

  // Make a second data delivery.
  estimated_capture_time +=
      source_bus->frames() * base::TimeDelta::FromSeconds(1) /
      source_params_.sample_rate();
  source_provider_->OnData(*source_bus, estimated_capture_time);

  // Verify that non-zero data samples are present in the results of the
  // following calls to provideInput().
  for (int i = 0; i < source_params_.frames_per_buffer();
       i += sink_params_.frames_per_buffer()) {
    sink_bus_->Zero();
    source_provider_->ProvideInput(audio_data,
                                   sink_params_.frames_per_buffer());
    EXPECT_NEAR(0.5f, sink_bus_->channel(0)[0], 0.001f);
    EXPECT_NEAR(0.5f, sink_bus_->channel(1)[0], 0.001f);
    EXPECT_DOUBLE_EQ(sink_bus_->channel(0)[0], sink_bus_->channel(1)[0]);
  }
}

TEST_F(WebRtcLocalAudioSourceProviderTest,
       DeleteSourceProviderBeforeStoppingTrack) {
  source_provider_.reset();

  // Stop the audio track.
  MediaStreamAudioTrack::From(blink_track_)->Stop();
}

TEST_F(WebRtcLocalAudioSourceProviderTest,
       StopTrackBeforeDeletingSourceProvider) {
  // Stop the audio track.
  MediaStreamAudioTrack::From(blink_track_)->Stop();

  // Delete the source provider.
  source_provider_.reset();
}

}  // namespace content

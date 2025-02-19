// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_MEDIA_CMA_BASE_VIDEO_PIPELINE_IMPL_H_
#define CHROMECAST_MEDIA_CMA_BASE_VIDEO_PIPELINE_IMPL_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "chromecast/media/cma/pipeline/av_pipeline_impl.h"
#include "chromecast/media/cma/pipeline/video_pipeline_client.h"
#include "chromecast/public/media/media_pipeline_backend.h"
#include "chromecast/public/media/stream_id.h"
#include "media/base/pipeline_status.h"

namespace media {
class AudioDecoderConfig;
class VideoDecoderConfig;
}

namespace chromecast {
struct Size;
namespace media {
class CodedFrameProvider;

class VideoPipelineImpl : public AvPipelineImpl {
 public:
  VideoPipelineImpl(MediaPipelineBackend::VideoDecoder* decoder,
                    const VideoPipelineClient& client);
  ~VideoPipelineImpl() override;

  ::media::PipelineStatus Initialize(
      const std::vector<::media::VideoDecoderConfig>& configs,
      std::unique_ptr<CodedFrameProvider> frame_provider);

  // AvPipelineImpl implementation:
  void UpdateStatistics() override;

 private:
  // AvPipelineImpl implementation:
  void OnVideoResolutionChanged(const Size& size) override;
  void OnUpdateConfig(StreamId id,
                      const ::media::AudioDecoderConfig& audio_config,
                      const ::media::VideoDecoderConfig& video_config) override;
  const EncryptionScheme& GetEncryptionScheme(StreamId id) const override;

  MediaPipelineBackend::VideoDecoder* const video_decoder_;
  const VideoPipelineClient::NaturalSizeChangedCB natural_size_changed_cb_;
  std::vector<EncryptionScheme> encryption_schemes_;

  DISALLOW_COPY_AND_ASSIGN(VideoPipelineImpl);
};

}  // namespace media
}  // namespace chromecast

#endif  // CHROMECAST_MEDIA_CMA_BASE_VIDEO_PIPELINE_IMPL_H_

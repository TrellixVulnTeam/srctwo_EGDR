// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/remoting/metrics.h"

#include "base/metrics/histogram_functions.h"
#include "base/metrics/histogram_macros.h"
#include "media/base/sample_rates.h"

namespace media {
namespace remoting {

namespace {

////////////////////////////////////////////////////////////////////////////////
// BEGIN: These were all borrowed from src/media/filters/ffmpeg_demuxer.cc.
// TODO(miu): This code will be de-duped in a soon-upcoming change.

// Some videos just want to watch the world burn, with a height of 0; cap the
// "infinite" aspect ratio resulting.
constexpr int kInfiniteRatio = 99999;

// Common aspect ratios (multiplied by 100 and truncated) used for histogramming
// video sizes.  These were taken on 20111103 from
// http://wikipedia.org/wiki/Aspect_ratio_(image)#Previous_and_currently_used_aspect_ratios
constexpr int kCommonAspectRatios100[] = {
    100, 115, 133, 137, 143, 150, 155, 160,  166,
    175, 177, 185, 200, 210, 220, 221, 235,  237,
    240, 255, 259, 266, 276, 293, 400, 1200, kInfiniteRatio,
};

// END: Code borrowed from src/media/filter/ffmpeg_demuxer.cc.
////////////////////////////////////////////////////////////////////////////////

// Buckets for video width histograms.
constexpr int kVideoWidthBuckets[] = {
    180,  240,  320,  480,  640,  720,  872,  940,   1280,
    1440, 1600, 1760, 1920, 2560, 3840, 7680, 16384,
};

}  // namespace

SessionMetricsRecorder::SessionMetricsRecorder()
    : last_audio_codec_(kUnknownAudioCodec),
      last_channel_layout_(CHANNEL_LAYOUT_NONE),
      last_sample_rate_(0),
      last_video_codec_(kUnknownVideoCodec),
      last_video_profile_(VIDEO_CODEC_PROFILE_UNKNOWN),
      remote_playback_is_disabled_(false) {}

SessionMetricsRecorder::~SessionMetricsRecorder() = default;

void SessionMetricsRecorder::WillStartSession(StartTrigger trigger) {
  DCHECK(!start_trigger_);
  start_trigger_ = trigger;
  start_time_ = base::TimeTicks::Now();
}

void SessionMetricsRecorder::DidStartSession() {
  UMA_HISTOGRAM_ENUMERATION("Media.Remoting.SessionStartTrigger",
                            *start_trigger_, START_TRIGGER_MAX + 1);
  if (last_audio_codec_ != kUnknownAudioCodec)
    RecordAudioConfiguration();
  if (last_video_codec_ != kUnknownVideoCodec)
    RecordVideoConfiguration();
  RecordTrackConfiguration();
}

void SessionMetricsRecorder::WillStopSession(StopTrigger trigger) {
  if (!start_trigger_)
    return;

  // Record what triggered the end of the session.
  UMA_HISTOGRAM_ENUMERATION("Media.Remoting.SessionStopTrigger", trigger,
                            STOP_TRIGGER_MAX + 1);

  // Record the session duration.
  const base::TimeDelta session_duration = base::TimeTicks::Now() - start_time_;
  UMA_HISTOGRAM_CUSTOM_TIMES("Media.Remoting.SessionDuration", session_duration,
                             base::TimeDelta::FromSeconds(15),
                             base::TimeDelta::FromHours(12), 50);

  // Reset |start_trigger_| since metrics recording of the current remoting
  // session has now completed.
  start_trigger_ = base::nullopt;
}

void SessionMetricsRecorder::OnPipelineMetadataChanged(
    const PipelineMetadata& metadata) {
  if (metadata.has_audio && metadata.audio_decoder_config.IsValidConfig()) {
    const auto& config = metadata.audio_decoder_config;
    // While in a remoting session, record audio configuration changes.
    const bool need_to_record_audio_configuration =
        start_trigger_ && (config.codec() != last_audio_codec_ ||
                           config.channel_layout() != last_channel_layout_ ||
                           config.samples_per_second() != last_sample_rate_);
    last_audio_codec_ = config.codec();
    last_channel_layout_ = config.channel_layout();
    last_sample_rate_ = config.samples_per_second();
    if (need_to_record_audio_configuration)
      RecordAudioConfiguration();
  } else {
    last_audio_codec_ = kUnknownAudioCodec;
    last_channel_layout_ = CHANNEL_LAYOUT_NONE;
    last_sample_rate_ = 0;
  }

  if (metadata.has_video && metadata.video_decoder_config.IsValidConfig()) {
    const auto& config = metadata.video_decoder_config;
    // While in a remoting session, record video configuration changes.
    const bool need_to_record_video_configuration =
        start_trigger_ && (config.codec() != last_video_codec_ ||
                           config.profile() != last_video_profile_ ||
                           metadata.natural_size != last_natural_size_);
    last_video_codec_ = config.codec();
    last_video_profile_ = config.profile();
    last_natural_size_ = metadata.natural_size;
    if (need_to_record_video_configuration)
      RecordVideoConfiguration();
  } else {
    last_video_codec_ = kUnknownVideoCodec;
    last_video_profile_ = VIDEO_CODEC_PROFILE_UNKNOWN;
    last_natural_size_ = gfx::Size();
  }

  // While in a remoting session, record whether audio or video media streams
  // started or ended.
  if (start_trigger_)
    RecordTrackConfiguration();
}

void SessionMetricsRecorder::OnRemotePlaybackDisabled(bool disabled) {
  if (disabled == remote_playback_is_disabled_)
    return;  // De-dupe redundant notifications.
  UMA_HISTOGRAM_BOOLEAN("Media.Remoting.AllowedByPage", !disabled);
  remote_playback_is_disabled_ = disabled;
}

void SessionMetricsRecorder::RecordMediaBitrateVersusCapacity(
    double kilobits_per_second,
    double capacity) {
  UMA_HISTOGRAM_CUSTOM_COUNTS("Media.Remoting.StartMediaBitrate",
                              kilobits_per_second, 1, 16 * 1024, 50);
  UMA_HISTOGRAM_CUSTOM_COUNTS("Media.Remoting.TransmissionCapacity", capacity,
                              1, 16 * 1024, 50);
  double remaining = capacity - kilobits_per_second;
  if (remaining >= 0) {
    UMA_HISTOGRAM_CUSTOM_COUNTS("Media.Remoting.CapacityOverMediaBitrate",
                                remaining, 1, 16 * 1024, 50);
  } else {
    UMA_HISTOGRAM_CUSTOM_COUNTS("Media.Remoting.MediaBitrateOverCapacity",
                                -remaining, 1, 16 * 1024, 50);
  }
}

void SessionMetricsRecorder::RecordAudioConfiguration() {
  UMA_HISTOGRAM_ENUMERATION("Media.Remoting.AudioCodec", last_audio_codec_,
                            kAudioCodecMax + 1);
  UMA_HISTOGRAM_ENUMERATION("Media.Remoting.AudioChannelLayout",
                            last_channel_layout_, CHANNEL_LAYOUT_MAX + 1);
  AudioSampleRate asr;
  if (ToAudioSampleRate(last_sample_rate_, &asr)) {
    UMA_HISTOGRAM_ENUMERATION("Media.Remoting.AudioSamplesPerSecond", asr,
                              kAudioSampleRateMax + 1);
  } else {
    UMA_HISTOGRAM_COUNTS_1M("Media.Remoting.AudioSamplesPerSecondUnexpected",
                            last_sample_rate_);
  }
}

void SessionMetricsRecorder::RecordVideoConfiguration() {
  UMA_HISTOGRAM_ENUMERATION("Media.Remoting.VideoCodec", last_video_codec_,
                            kVideoCodecMax + 1);
  UMA_HISTOGRAM_ENUMERATION("Media.Remoting.VideoCodecProfile",
                            last_video_profile_, VIDEO_CODEC_PROFILE_MAX + 1);
  UMA_HISTOGRAM_CUSTOM_ENUMERATION(
      "Media.Remoting.VideoNaturalWidth", last_natural_size_.width(),
      base::CustomHistogram::ArrayToCustomRanges(
          kVideoWidthBuckets, arraysize(kVideoWidthBuckets)));
  // Intentionally use integer division to truncate the result.
  const int aspect_ratio_100 =
      last_natural_size_.height()
          ? (last_natural_size_.width() * 100) / last_natural_size_.height()
          : kInfiniteRatio;
  UMA_HISTOGRAM_CUSTOM_ENUMERATION(
      "Media.Remoting.VideoAspectRatio", aspect_ratio_100,
      base::CustomHistogram::ArrayToCustomRanges(
          kCommonAspectRatios100, arraysize(kCommonAspectRatios100)));
}

void SessionMetricsRecorder::RecordTrackConfiguration() {
  TrackConfiguration config = NEITHER_AUDIO_NOR_VIDEO;
  if (last_audio_codec_ != kUnknownAudioCodec)
    config = AUDIO_ONLY;
  if (last_video_codec_ != kUnknownVideoCodec) {
    if (config == AUDIO_ONLY)
      config = AUDIO_AND_VIDEO;
    else
      config = VIDEO_ONLY;
  }
  UMA_HISTOGRAM_ENUMERATION("Media.Remoting.TrackConfiguration", config,
                            TRACK_CONFIGURATION_MAX + 1);
}

RendererMetricsRecorder::RendererMetricsRecorder()
    : start_time_(base::TimeTicks::Now()), did_record_first_playout_(false) {}

RendererMetricsRecorder::~RendererMetricsRecorder() = default;

void RendererMetricsRecorder::OnRendererInitialized() {
  const base::TimeDelta elapsed_since_start =
      base::TimeTicks::Now() - start_time_;
  UMA_HISTOGRAM_CUSTOM_TIMES("Media.Remoting.TimeUntilRemoteInitialized",
                             elapsed_since_start,
                             base::TimeDelta::FromMilliseconds(10),
                             base::TimeDelta::FromSeconds(30), 50);
}

void RendererMetricsRecorder::OnEvidenceOfPlayoutAtReceiver() {
  if (did_record_first_playout_)
    return;
  const base::TimeDelta elapsed_since_start =
      base::TimeTicks::Now() - start_time_;
  UMA_HISTOGRAM_CUSTOM_TIMES("Media.Remoting.TimeUntilFirstPlayout",
                             elapsed_since_start,
                             base::TimeDelta::FromMilliseconds(10),
                             base::TimeDelta::FromSeconds(30), 50);
  did_record_first_playout_ = true;
}

void RendererMetricsRecorder::OnAudioRateEstimate(int kilobits_per_second) {
  UMA_HISTOGRAM_CUSTOM_COUNTS("Media.Remoting.AudioBitrate",
                              kilobits_per_second, 1, 1024, 50);
}

void RendererMetricsRecorder::OnVideoRateEstimate(int kilobits_per_second) {
  UMA_HISTOGRAM_CUSTOM_COUNTS("Media.Remoting.VideoBitrate",
                              kilobits_per_second, 1, 16 * 1024, 50);
}

}  // namespace remoting
}  // namespace media

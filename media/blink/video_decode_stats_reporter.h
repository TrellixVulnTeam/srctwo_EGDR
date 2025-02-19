// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_BLINK_MEDIA_CAPABILITIES_REPORTER_H_
#define MEDIA_BLINK_MEDIA_CAPABILITIES_REPORTER_H_

#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/time/default_tick_clock.h"
#include "base/time/tick_clock.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "media/base/pipeline_status.h"
#include "media/base/video_decoder_config.h"
#include "media/blink/media_blink_export.h"
#include "media/mojo/interfaces/video_decode_stats_recorder.mojom.h"

#include <memory>

namespace media {

// Reports on playback smoothness and for a given video codec profile, natural
// size, and fps. When these properties change the current report will be
// finalized and a new report will be started. Ongoing reports are also
// finalized at destruction and process tear-down.
class MEDIA_BLINK_EXPORT VideoDecodeStatsReporter {
 public:
  using GetPipelineStatsCB = base::Callback<PipelineStatistics(void)>;

  VideoDecodeStatsReporter(mojom::VideoDecodeStatsRecorderPtr recorder_ptr,
                           GetPipelineStatsCB get_pipeline_stats_cb,
                           const VideoDecoderConfig& video_config,
                           std::unique_ptr<base::TickClock> tick_clock =
                               base::MakeUnique<base::DefaultTickClock>());
  ~VideoDecodeStatsReporter();

  void OnPlaying();
  void OnPaused();
  void OnHidden();
  void OnShown();
  void OnNaturalSizeChanged(const gfx::Size& natural_size);
  void OnVideoConfigChanged(const VideoDecoderConfig& video_config);

  // NOTE: We do not listen for playback rate changes. These implicitly change
  // the frame rate and surface via PipelineStatistics'
  // video_frame_duration_average.

 private:
  // Friends so it can see the static constants and inspect when the timer is
  // running / should be running.
  friend class VideoDecodeStatsReporterTest;

  // Constants placed in header file for test visibility.
  enum : int {
    // Timer interval for recording stats when frame rate and other stream
    // properties are steady.
    kRecordingIntervalMs = 2000,

    // FrameRates must remain stable for a duration greater than this amount to
    // avoid being classified as a "tiny fps window". See |kMaxTinyFpsWindows|.
    kTinyFpsWindowMs = 5000,

    // Limits the number of consecutive "tiny fps windows", as defined by
    // |kTinyFpsWindowMs|. If this limit is met, we will give up until some
    // stream property (e.g. decoder config) changes before trying again. We do
    // not wish to record stats for variable frame rate content.
    kMaxTinyFpsWindows = 5,

    // Number of consecutive samples that must bucket to the same frame rate in
    // order for frame rate to be considered "stable" enough to start recording
    // stats.
    kRequiredStableFpsSamples = 5,

    // Limits the number attempts to detect stable frame rate. When this limit
    // is reached, we will give up until some stream property (e.g. decoder
    // config) changes before trying again. We do not wish to record stats for
    // variable frame rate content.
    kMaxUnstableFpsChanges = 10,
  };

  // Main driver of report updates. Queries |get_pipeline_stats_cb_| for the
  // latest stats. Detects frame rate changes and playback stalls. Sends stats
  // the VideoDecodeStatsRecorder (browser process) for saving. Should only be
  // called when ShouldBeReporting() == true.
  void UpdateStats();

  // Round |raw_fps| to the nearest (smaller or larger) "bucket". FrameRates in
  // the same bucket should have nearly identical decode performance
  // characteristics. Bucketing helps avoid fragmentation of recorded stats.
  int GetFpsBucket(double raw_fps) const;

  // Find the nearest "bucket" with dimensions >= |raw_size|. While smaller
  // buckets may more closely describe |raw_size|, the next largest bucket is
  // chosen to surface cutoff resolutions in HW-accelerated decoders. Exceeding
  // the HW cutoff will invoke the software fallback, giving potentially very
  // different decode performance at larger resolutions. Will return an empty
  // size if |raw_size| is too small to be bucketed.
  gfx::Size GetSizeBucket(const gfx::Size& raw_size) const;

  // Run |stats_cb_timer_| at the specified |interval|. If the timer is already
  // running, any existing callbacks will be canceled/delayed until
  // |interval| has elapsed.
  void RunStatsTimerAtInterval(base::TimeDelta interval);

  // Called to begin a new report following changes to stream metadata (e.g.
  // natural size). Arguments used to update |freames_decoded_offset_| and
  // |frames_dropped_offset_| so that frame counts for this report begin at 0.
  void StartNewRecord(uint32_t frames_decoded_offset,
                      uint32_t frames_dropped_offset);

  // Reset frame rate tracking state to force a fresh attempt at detection. When
  // a stable frame rate is successfully detected, UpdateStats() will begin a
  // new record will begin with the detected frame rate. Note: callers must
  // separately ensure the |stats_cb_timer_| is running for frame rate detection
  // to occur.
  void ResetFrameRateState();

  // Called by UpdateStats() to verify decode is progressing and sanity check
  // decode/dropped frame counts. Will manage decoded/dropped frame state and
  // relax timer when no decode progress is made. Returns true iff decode is
  // progressing.
  bool UpdateDecodeProgress(const PipelineStatistics& stats);

  // Called by UpdateStats() to do frame rate detection. Will manage frame rate
  // state, stats timer, and will start new capabilities records when frame rate
  // changes. Returns true iff frame rate is stable.
  bool UpdateFrameRateStability(const PipelineStatistics& stats);

  // Returns true if the |stats_timer_cb_| should be running. Should be called
  // after any state change (e.g. |is_playing_|) as a check on whether to start
  // the the timer.
  bool ShouldBeReporting() const;

  // TimeDelta wrappers around |kRecordingIntervalMs| and |kTimyFpsWindowMs|.
  // Defined as a class member to avoid static initialization.
  // TODO(chcunningham): convert to static constexpr when MSVC support arrives.
  const base::TimeDelta kRecordingInterval;
  const base::TimeDelta kTinyFpsWindowDuration;

  // Pointer to the remote recorder. The recorder runs in the browser process
  // and finalizes the record in the event of fast render process tear down.
  mojom::VideoDecodeStatsRecorderPtr recorder_ptr_;

  // Callback for retrieving playback statistics.
  GetPipelineStatsCB get_pipeline_stats_cb_;

  // Latest video decoder config, provided at construction or by
  // OnVideoConfigChanged(). Used to determine the codec, profile, and natural
  // size. Note that |video_config_.natural_size()| may differ slightly from the
  // stored |natural_size_| due to size bucketing. See GetSizeBucket()
  VideoDecoderConfig video_config_;

  // Last natural size, either from the latest |video_config_|, or from
  // OnNaturalSizechanged(). The dimensions of this size will always be rounded
  // to the nearest size bucket. If the original size is very small, the
  // bucketed size will simply be empty. See GetSizeBucket().
  gfx::Size natural_size_;

  // Clock for |stats_cb_timer_| and getting current tick count (NowTicks()).
  // Tests may supply a mock clock via the constructor.
  std::unique_ptr<base::TickClock> tick_clock_;

  // Timer for all stats callbacks. Timer interval will be dynamically set based
  // on state of reporter. See calls to RunStatsTimerAtIntervalMs().
  base::RepeatingTimer stats_cb_timer_;

  // Latest frame duration bucketed into one of kFrameRateBuckets.
  int last_observed_fps_ = 0;

  // Count of consecutive samples with frame duration matching
  // |last_observed_fps_|.
  int num_stable_fps_samples_ = 0;

  // Count of consecutive samples with frame duration NOT matching
  // |last_obseved_fps_|. Used to throttle/limit attempts to stabilize FPS since
  // some videos may have variable frame rate.
  int num_unstable_fps_changes_ = 0;

  // Count of consecutive stable FPS windows, where stable FPS was detected but
  // it lasted for a very short duration before changing again.
  int num_consecutive_tiny_fps_windows_ = 0;

  // True whenever we fail to determine a stable FPS, or if windows of stable
  // FPS are too tiny to be useful.
  bool fps_stabilization_failed_ = false;

  // Tick time of the most recent FPS stabilization. When FPS changes, this time
  // is compared to TimeTicks::Now() to approximate the duration of the last
  // stable FPS window.
  base::TimeTicks last_fps_stabilized_ticks_;

  // Count of frames decoded observed in last pipeline stats update. Used to
  // check whether decode/playback has actually advanced.
  uint32_t last_frames_decoded_ = 0;

  // Count of frames dropped observed in last pipeline stats update. Used to
  // verify that count never decreases.
  uint32_t last_frames_dropped_ = 0;

  // Notes the number of frames decoded at the start of the current video
  // configuration (profile, resolution, fps). Should be subtracted from
  // pipeline frames decoded stats before sending to recorder.
  uint32_t frames_decoded_offset_ = 0;

  // Notes the number of frames dropped at the start of the current video
  // configuration (profile, resolution, fps). Should be subtracted from
  // pipeline frames dropped stats before sending to recorder.
  uint32_t frames_dropped_offset_ = 0;

  // Set true by OnPlaying(), false by OnPaused(). We should not run the
  // |stats_cb_timer_| when not playing.
  bool is_playing_ = false;

  // Set true by OnHidden(), false by OnVisible(). We should not run the
  // |stats_cb_timer_| when player is backgrounded.
  bool is_backgrounded_ = false;

  DISALLOW_COPY_AND_ASSIGN(VideoDecodeStatsReporter);
};

}  // namespace media

#endif  // MEDIA_BLINK_MEDIA_CAPABILITIES_REPORTER_H_
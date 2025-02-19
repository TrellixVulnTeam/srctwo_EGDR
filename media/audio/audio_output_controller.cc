// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/audio/audio_output_controller.h"

#include <stdint.h>

#include <algorithm>
#include <limits>

#include "base/bind.h"
#include "base/metrics/histogram_macros.h"
#include "base/numerics/safe_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/task_runner_util.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"
#include "base/trace_event/trace_event.h"
#include "media/base/audio_timestamp_helper.h"

using base::TimeDelta;

namespace media {
namespace {
// Time in seconds between two successive measurements of audio power levels.
constexpr int kPowerMonitorLogIntervalSeconds = 15;
}  // namespace

AudioOutputController::AudioOutputController(
    AudioManager* audio_manager,
    EventHandler* handler,
    const AudioParameters& params,
    const std::string& output_device_id,
    SyncReader* sync_reader)
    : audio_manager_(audio_manager),
      params_(params),
      handler_(handler),
      output_device_id_(output_device_id),
      stream_(NULL),
      diverting_to_stream_(NULL),
      should_duplicate_(0),
      volume_(1.0),
      state_(kEmpty),
      sync_reader_(sync_reader),
      message_loop_(audio_manager->GetTaskRunner()),
      power_monitor_(
          params.sample_rate(),
          TimeDelta::FromMilliseconds(kPowerMeasurementTimeConstantMillis)),
      on_more_io_data_called_(0),
      weak_factory_for_errors_(this) {
  DCHECK(audio_manager);
  DCHECK(handler_);
  DCHECK(sync_reader_);
  DCHECK(message_loop_.get());
  weak_this_for_errors_ = weak_factory_for_errors_.GetWeakPtr();
}

AudioOutputController::~AudioOutputController() {
  CHECK_EQ(kClosed, state_);
  CHECK_EQ(nullptr, stream_);
  CHECK(duplication_targets_.empty());
}

// static
scoped_refptr<AudioOutputController> AudioOutputController::Create(
    AudioManager* audio_manager,
    EventHandler* event_handler,
    const AudioParameters& params,
    const std::string& output_device_id,
    SyncReader* sync_reader) {
  CHECK(audio_manager);
  CHECK_EQ(AudioManager::Get(), audio_manager);
  DCHECK(sync_reader);
  DCHECK(params.IsValid());

  scoped_refptr<AudioOutputController> controller(new AudioOutputController(
      audio_manager, event_handler, params, output_device_id, sync_reader));
  controller->message_loop_->PostTask(
      FROM_HERE,
      base::BindOnce(&AudioOutputController::DoCreate, controller, false));
  return controller;
}

void AudioOutputController::Play() {
  CHECK_EQ(AudioManager::Get(), audio_manager_);
  message_loop_->PostTask(FROM_HERE,
                          base::BindOnce(&AudioOutputController::DoPlay, this));
}

void AudioOutputController::Pause() {
  CHECK_EQ(AudioManager::Get(), audio_manager_);
  message_loop_->PostTask(
      FROM_HERE, base::BindOnce(&AudioOutputController::DoPause, this));
}

void AudioOutputController::Close(base::OnceClosure closed_task) {
  CHECK_EQ(AudioManager::Get(), audio_manager_);
  DCHECK(!closed_task.is_null());
  message_loop_->PostTaskAndReply(
      FROM_HERE, base::BindOnce(&AudioOutputController::DoClose, this),
      std::move(closed_task));
}

void AudioOutputController::SetVolume(double volume) {
  CHECK_EQ(AudioManager::Get(), audio_manager_);
  message_loop_->PostTask(
      FROM_HERE,
      base::BindOnce(&AudioOutputController::DoSetVolume, this, volume));
}

void AudioOutputController::DoCreate(bool is_for_device_change) {
  DCHECK(message_loop_->BelongsToCurrentThread());
  SCOPED_UMA_HISTOGRAM_TIMER("Media.AudioOutputController.CreateTime");
  TRACE_EVENT0("audio", "AudioOutputController::DoCreate");
  handler_->OnLog(base::StringPrintf("AOC::DoCreate (for device change: %s)",
                                     is_for_device_change ? "yes" : "no"));

  // Close() can be called before DoCreate() is executed.
  if (state_ == kClosed)
    return;

  DoStopCloseAndClearStream();  // Calls RemoveOutputDeviceChangeListener().
  DCHECK_EQ(kEmpty, state_);

  stream_ = diverting_to_stream_ ?
      diverting_to_stream_ :
      audio_manager_->MakeAudioOutputStreamProxy(params_, output_device_id_);
  if (!stream_) {
    state_ = kError;
    handler_->OnControllerError();
    return;
  }

  if (!stream_->Open()) {
    DoStopCloseAndClearStream();
    state_ = kError;
    handler_->OnControllerError();
    return;
  }

  // Everything started okay, so re-register for state change callbacks if
  // stream_ was created via AudioManager.
  if (stream_ != diverting_to_stream_)
    audio_manager_->AddOutputDeviceChangeListener(this);

  // We have successfully opened the stream. Set the initial volume.
  stream_->SetVolume(volume_);

  // Finally set the state to kCreated.
  state_ = kCreated;

  // And then report we have been created if we haven't done so already.
  if (!is_for_device_change)
    handler_->OnControllerCreated();
}

void AudioOutputController::DoPlay() {
  DCHECK(message_loop_->BelongsToCurrentThread());
  SCOPED_UMA_HISTOGRAM_TIMER("Media.AudioOutputController.PlayTime");
  TRACE_EVENT0("audio", "AudioOutputController::DoPlay");
  handler_->OnLog("AOC::DoPlay");

  // We can start from created or paused state.
  if (state_ != kCreated && state_ != kPaused)
    return;

  // Ask for first packet.
  sync_reader_->RequestMoreData(base::TimeDelta(), base::TimeTicks(), 0);

  state_ = kPlaying;

  if (will_monitor_audio_levels()) {
    last_audio_level_log_time_ = base::TimeTicks::Now();
  }

  stream_->Start(this);

  // For UMA tracking purposes, start the wedge detection timer.  This allows us
  // to record statistics about the number of wedged playbacks in the field.
  //
  // WedgeCheck() will look to see if |on_more_io_data_called_| is true after
  // the timeout expires.  Care must be taken to ensure the wedge check delay is
  // large enough that the value isn't queried while OnMoreDataIO() is setting
  // it.
  //
  // Timer self-manages its lifetime and WedgeCheck() will only record the UMA
  // statistic if state is still kPlaying.  Additional Start() calls will
  // invalidate the previous timer.
  wedge_timer_.reset(new base::OneShotTimer());
  wedge_timer_->Start(
      FROM_HERE, TimeDelta::FromSeconds(5), this,
      &AudioOutputController::WedgeCheck);

  handler_->OnControllerPlaying();
}

void AudioOutputController::StopStream() {
  DCHECK(message_loop_->BelongsToCurrentThread());

  if (state_ == kPlaying) {
    wedge_timer_.reset();
    stream_->Stop();

    if (will_monitor_audio_levels()) {
      LogAudioPowerLevel("StopStream");
    }

    // A stopped stream is silent, and power_montior_.Scan() is no longer being
    // called; so we must reset the power monitor.
    power_monitor_.Reset();

    state_ = kPaused;
  }
}

void AudioOutputController::DoPause() {
  DCHECK(message_loop_->BelongsToCurrentThread());
  SCOPED_UMA_HISTOGRAM_TIMER("Media.AudioOutputController.PauseTime");
  TRACE_EVENT0("audio", "AudioOutputController::DoPause");
  handler_->OnLog("AOC::DoPause");

  StopStream();

  if (state_ != kPaused)
    return;

  // Let the renderer know we've stopped.  Necessary to let PPAPI clients know
  // audio has been shutdown.  TODO(dalecurtis): This stinks.  PPAPI should have
  // a better way to know when it should exit PPB_Audio_Shared::Run().
  sync_reader_->RequestMoreData(base::TimeDelta::Max(), base::TimeTicks(), 0);

  handler_->OnControllerPaused();
}

void AudioOutputController::DoClose() {
  DCHECK(message_loop_->BelongsToCurrentThread());
  SCOPED_UMA_HISTOGRAM_TIMER("Media.AudioOutputController.CloseTime");
  TRACE_EVENT0("audio", "AudioOutputController::DoClose");
  handler_->OnLog("AOC::DoClose");

  if (state_ != kClosed) {
    DoStopCloseAndClearStream();
    sync_reader_->Close();
    state_ = kClosed;
  }
}

void AudioOutputController::DoSetVolume(double volume) {
  DCHECK(message_loop_->BelongsToCurrentThread());

  // Saves the volume to a member first. We may not be able to set the volume
  // right away but when the stream is created we'll set the volume.
  volume_ = volume;

  switch (state_) {
    case kCreated:
    case kPlaying:
    case kPaused:
      stream_->SetVolume(volume_);
      break;
    default:
      return;
  }
}

void AudioOutputController::DoReportError() {
  DCHECK(message_loop_->BelongsToCurrentThread());
  if (state_ != kClosed)
    handler_->OnControllerError();
}

int AudioOutputController::OnMoreData(base::TimeDelta delay,
                                      base::TimeTicks delay_timestamp,
                                      int prior_frames_skipped,
                                      AudioBus* dest) {
  TRACE_EVENT0("audio", "AudioOutputController::OnMoreData");

  // Indicate that we haven't wedged (at least not indefinitely, WedgeCheck()
  // may have already fired if OnMoreData() took an abnormal amount of time).
  // Since this thread is the only writer of |on_more_io_data_called_| once the
  // thread starts, its safe to compare and then increment.
  if (on_more_io_data_called_.IsZero())
    on_more_io_data_called_.Increment();

  sync_reader_->Read(dest);

  const int frames =
      dest->is_bitstream_format() ? dest->GetBitstreamFrames() : dest->frames();
  delay += AudioTimestampHelper::FramesToTime(frames, params_.sample_rate());

  sync_reader_->RequestMoreData(delay, delay_timestamp, prior_frames_skipped);

  if (should_duplicate_.IsOne()) {
    const base::TimeTicks reference_time = delay_timestamp + delay;
    std::unique_ptr<AudioBus> copy(AudioBus::Create(params_));
    dest->CopyTo(copy.get());
    message_loop_->PostTask(
        FROM_HERE,
        base::BindOnce(
            &AudioOutputController::BroadcastDataToDuplicationTargets, this,
            std::move(copy), reference_time));
  }

  if (will_monitor_audio_levels()) {
    power_monitor_.Scan(*dest, frames);

    const auto now = base::TimeTicks::Now();
    if ((now - last_audio_level_log_time_).InSeconds() >
        kPowerMonitorLogIntervalSeconds) {
      LogAudioPowerLevel("OnMoreData");
      last_audio_level_log_time_ = now;
    }
  }

  return frames;
}

void AudioOutputController::BroadcastDataToDuplicationTargets(
    std::unique_ptr<AudioBus> audio_bus,
    base::TimeTicks reference_time) {
  DCHECK(message_loop_->BelongsToCurrentThread());
  if (state_ != kPlaying || duplication_targets_.empty())
    return;

  // Note: Do not need to acquire lock since this is running on the same thread
  // as where the set is modified.
  for (auto target = std::next(duplication_targets_.begin(), 1);
       target != duplication_targets_.end(); ++target) {
    std::unique_ptr<AudioBus> copy(AudioBus::Create(params_));
    audio_bus->CopyTo(copy.get());
    (*target)->OnData(std::move(copy), reference_time);
  }

  (*duplication_targets_.begin())->OnData(std::move(audio_bus), reference_time);
}

void AudioOutputController::LogAudioPowerLevel(const std::string& call_name) {
  std::pair<float, bool> power_and_clip =
      power_monitor_.ReadCurrentPowerAndClip();
  handler_->OnLog(base::StringPrintf("AOC::%s: average audio level=%.2f dBFS",
                                     call_name.c_str(), power_and_clip.first));
}

void AudioOutputController::OnError() {
  // Handle error on the audio controller thread.  We defer errors for one
  // second in case they are the result of a device change; delay chosen to
  // exceed duration of device changes which take a few hundred milliseconds.
  message_loop_->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(&AudioOutputController::DoReportError,
                     weak_this_for_errors_),
      base::TimeDelta::FromSeconds(1));
}

void AudioOutputController::DoStopCloseAndClearStream() {
  DCHECK(message_loop_->BelongsToCurrentThread());

  // Allow calling unconditionally and bail if we don't have a stream_ to close.
  if (stream_) {
    // Ensure no errors will be delivered while we cycle streams and any that
    // occurred immediately prior to the device change are dropped.
    weak_factory_for_errors_.InvalidateWeakPtrs();

    // De-register from state change callbacks if stream_ was created via
    // AudioManager.
    if (stream_ != diverting_to_stream_)
      audio_manager_->RemoveOutputDeviceChangeListener(this);

    StopStream();
    stream_->Close();

    if (stream_ == diverting_to_stream_)
      diverting_to_stream_ = NULL;
    stream_ = NULL;

    // Since the stream is stopped, we can now update |weak_this_for_errors_|.
    weak_this_for_errors_ = weak_factory_for_errors_.GetWeakPtr();
  }

  state_ = kEmpty;
}

void AudioOutputController::OnDeviceChange() {
  DCHECK(message_loop_->BelongsToCurrentThread());
  SCOPED_UMA_HISTOGRAM_TIMER("Media.AudioOutputController.DeviceChangeTime");
  TRACE_EVENT0("audio", "AudioOutputController::OnDeviceChange");

  auto state_to_string = [](State state) {
    switch (state) {
      case AudioOutputController::kEmpty:
        return "empty";
      case AudioOutputController::kCreated:
        return "created";
      case AudioOutputController::kPlaying:
        return "playing";
      case AudioOutputController::kPaused:
        return "paused";
      case AudioOutputController::kClosed:
        return "closed";
      case AudioOutputController::kError:
        return "error";
    };
    return "unknown";
  };

  handler_->OnLog(base::StringPrintf("AOC::OnDeviceChange while in state: %s",
                                     state_to_string(state_)));

  // TODO(dalecurtis): Notify the renderer side that a device change has
  // occurred.  Currently querying the hardware information here will lead to
  // crashes on OSX.  See http://crbug.com/158170.

  // Recreate the stream (DoCreate() will first shut down an existing stream).
  // Exit if we ran into an error.
  const State original_state = state_;
  DoCreate(true);
  if (!stream_ || state_ == kError)
    return;

  // Get us back to the original state or an equivalent state.
  switch (original_state) {
    case kPlaying:
      DoPlay();
      return;
    case kCreated:
    case kPaused:
      // From the outside these two states are equivalent.
      return;
    default:
      NOTREACHED() << "Invalid original state.";
  }
}

const AudioParameters& AudioOutputController::GetAudioParameters() {
  return params_;
}

void AudioOutputController::StartDiverting(AudioOutputStream* to_stream) {
  message_loop_->PostTask(
      FROM_HERE, base::BindOnce(&AudioOutputController::DoStartDiverting, this,
                                to_stream));
}

void AudioOutputController::StopDiverting() {
  message_loop_->PostTask(
      FROM_HERE, base::BindOnce(&AudioOutputController::DoStopDiverting, this));
}

void AudioOutputController::StartDuplicating(AudioPushSink* sink) {
  message_loop_->PostTask(
      FROM_HERE,
      base::BindOnce(&AudioOutputController::DoStartDuplicating, this, sink));
}

void AudioOutputController::StopDuplicating(AudioPushSink* sink) {
  message_loop_->PostTask(
      FROM_HERE,
      base::BindOnce(&AudioOutputController::DoStopDuplicating, this, sink));
}

void AudioOutputController::DoStartDiverting(AudioOutputStream* to_stream) {
  DCHECK(message_loop_->BelongsToCurrentThread());

  if (state_ == kClosed)
    return;

  DCHECK(!diverting_to_stream_);
  diverting_to_stream_ = to_stream;
  // Note: OnDeviceChange() will engage the "re-create" process, which will
  // detect and use the alternate AudioOutputStream rather than create a new one
  // via AudioManager.
  OnDeviceChange();
}

void AudioOutputController::DoStopDiverting() {
  DCHECK(message_loop_->BelongsToCurrentThread());

  if (state_ == kClosed)
    return;

  // Note: OnDeviceChange() will cause the existing stream (the consumer of the
  // diverted audio data) to be closed, and diverting_to_stream_ will be set
  // back to NULL.
  OnDeviceChange();
  DCHECK(!diverting_to_stream_);
}

void AudioOutputController::DoStartDuplicating(AudioPushSink* to_stream) {
  DCHECK(message_loop_->BelongsToCurrentThread());
  if (state_ == kClosed)
    return;

  if (duplication_targets_.empty())
    should_duplicate_.Increment();

  duplication_targets_.insert(to_stream);
}

void AudioOutputController::DoStopDuplicating(AudioPushSink* to_stream) {
  DCHECK(message_loop_->BelongsToCurrentThread());
  to_stream->Close();

  duplication_targets_.erase(to_stream);
  if (duplication_targets_.empty()) {
    const bool is_nonzero = should_duplicate_.Decrement();
    DCHECK(!is_nonzero);
  }
}

std::pair<float, bool> AudioOutputController::ReadCurrentPowerAndClip() {
  DCHECK(will_monitor_audio_levels());
  return power_monitor_.ReadCurrentPowerAndClip();
}

void AudioOutputController::WedgeCheck() {
  DCHECK(message_loop_->BelongsToCurrentThread());

  // If we should be playing and we haven't, that's a wedge.
  if (state_ == kPlaying) {
    UMA_HISTOGRAM_BOOLEAN("Media.AudioOutputControllerPlaybackStartupSuccess",
                          on_more_io_data_called_.IsOne());
  }
}

}  // namespace media

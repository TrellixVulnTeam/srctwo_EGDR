// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/blink/video_frame_compositor.h"

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/message_loop/message_loop.h"
#include "base/time/default_tick_clock.h"
#include "base/trace_event/auto_open_close_event.h"
#include "base/trace_event/trace_event.h"
#include "media/base/media_switches.h"
#include "media/base/video_frame.h"
#include "third_party/WebKit/public/platform/WebVideoFrameSubmitter.h"

namespace media {

// Amount of time to wait between UpdateCurrentFrame() callbacks before starting
// background rendering to keep the Render() callbacks moving.
const int kBackgroundRenderingTimeoutMs = 250;

VideoFrameCompositor::VideoFrameCompositor(
    const scoped_refptr<base::SingleThreadTaskRunner>& task_runner)
    : task_runner_(task_runner),
      tick_clock_(new base::DefaultTickClock()),
      background_rendering_enabled_(true),
      background_rendering_timer_(
          FROM_HERE,
          base::TimeDelta::FromMilliseconds(kBackgroundRenderingTimeoutMs),
          base::Bind(&VideoFrameCompositor::BackgroundRender,
                     base::Unretained(this)),
          // Task is not repeating, CallRender() will reset the task as needed.
          false),
      client_(nullptr),
      rendering_(false),
      rendered_last_frame_(false),
      is_background_rendering_(false),
      new_background_frame_(false),
      // Assume 60Hz before the first UpdateCurrentFrame() call.
      last_interval_(base::TimeDelta::FromSecondsD(1.0 / 60)),
      callback_(nullptr),
      surface_layer_for_video_enabled_(
          base::FeatureList::IsEnabled(media::kUseSurfaceLayerForVideo)) {
  background_rendering_timer_.SetTaskRunner(task_runner_);

  if (surface_layer_for_video_enabled_)
    submitter_ = blink::WebVideoFrameSubmitter::Create(this);
}

VideoFrameCompositor::~VideoFrameCompositor() {
  DCHECK(task_runner_->BelongsToCurrentThread());
  DCHECK(!callback_);
  DCHECK(!rendering_);
  if (client_)
    client_->StopUsingProvider();
  if (submitter_)
    submitter_->StopUsingProvider();
}

void VideoFrameCompositor::EnableSubmission(const viz::FrameSinkId& id) {
  DCHECK(task_runner_->BelongsToCurrentThread());
  submitter_->StartSubmitting(id);
}

bool VideoFrameCompositor::IsClientSinkAvailable() {
  DCHECK(task_runner_->BelongsToCurrentThread());
  return !surface_layer_for_video_enabled_ ? client_ != nullptr
      : submitter_.get() != nullptr;
}

void VideoFrameCompositor::OnRendererStateUpdate(bool new_state) {
  DCHECK(task_runner_->BelongsToCurrentThread());
  DCHECK_NE(rendering_, new_state);
  rendering_ = new_state;

  if (!auto_open_close_) {
    auto_open_close_.reset(new base::trace_event::AutoOpenCloseEvent(
        base::trace_event::AutoOpenCloseEvent::Type::ASYNC, "media,rail",
        "VideoPlayback"));
  }

  if (rendering_) {
    auto_open_close_->Begin();
  } else {
    new_processed_frame_cb_.Reset();
    auto_open_close_->End();
  }

  if (rendering_) {
    // Always start playback in background rendering mode, if |client_| kicks
    // in right away it's okay.
    BackgroundRender();
  } else if (background_rendering_enabled_) {
    background_rendering_timer_.Stop();
  } else {
    DCHECK(!background_rendering_timer_.IsRunning());
  }

  if (!IsClientSinkAvailable())
    return;

  if (surface_layer_for_video_enabled_) {
    if (rendering_)
      submitter_->StartRendering();
    else
      submitter_->StopRendering();
  } else {
    if (rendering_)
      client_->StartRendering();
    else
      client_->StopRendering();
  }
}

void VideoFrameCompositor::SetVideoFrameProviderClient(
    cc::VideoFrameProvider::Client* client) {
  DCHECK(task_runner_->BelongsToCurrentThread());
  if (client_)
    client_->StopUsingProvider();
  client_ = client;

  // |client_| may now be null, so verify before calling it.
  if (rendering_ && client_)
    client_->StartRendering();
}

scoped_refptr<VideoFrame> VideoFrameCompositor::GetCurrentFrame() {
  DCHECK(task_runner_->BelongsToCurrentThread());
  return current_frame_;
}

void VideoFrameCompositor::PutCurrentFrame() {
  DCHECK(task_runner_->BelongsToCurrentThread());
  rendered_last_frame_ = true;
}

bool VideoFrameCompositor::UpdateCurrentFrame(base::TimeTicks deadline_min,
                                              base::TimeTicks deadline_max) {
  DCHECK(task_runner_->BelongsToCurrentThread());
  return CallRender(deadline_min, deadline_max, false);
}

bool VideoFrameCompositor::HasCurrentFrame() {
  DCHECK(task_runner_->BelongsToCurrentThread());
  return static_cast<bool>(current_frame_);
}

void VideoFrameCompositor::Start(RenderCallback* callback) {
  // Called from the media thread, so acquire the callback under lock before
  // returning in case a Stop() call comes in before the PostTask is processed.
  base::AutoLock lock(callback_lock_);
  DCHECK(!callback_);
  callback_ = callback;
  task_runner_->PostTask(
      FROM_HERE, base::Bind(&VideoFrameCompositor::OnRendererStateUpdate,
                            base::Unretained(this), true));
}

void VideoFrameCompositor::Stop() {
  // Called from the media thread, so release the callback under lock before
  // returning to avoid a pending UpdateCurrentFrame() call occurring before
  // the PostTask is processed.
  base::AutoLock lock(callback_lock_);
  DCHECK(callback_);
  callback_ = nullptr;
  task_runner_->PostTask(
      FROM_HERE, base::Bind(&VideoFrameCompositor::OnRendererStateUpdate,
                            base::Unretained(this), false));
}

void VideoFrameCompositor::PaintSingleFrame(
    const scoped_refptr<VideoFrame>& frame,
    bool repaint_duplicate_frame) {
  if (!task_runner_->BelongsToCurrentThread()) {
    task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&VideoFrameCompositor::PaintSingleFrame,
                   base::Unretained(this), frame, repaint_duplicate_frame));
    return;
  }
  if (ProcessNewFrame(frame, repaint_duplicate_frame) &&
      IsClientSinkAvailable()) {
    if (!surface_layer_for_video_enabled_)
      client_->DidReceiveFrame();
    else
      submitter_->DidReceiveFrame();
  }
}

scoped_refptr<VideoFrame>
VideoFrameCompositor::GetCurrentFrameAndUpdateIfStale() {
  DCHECK(task_runner_->BelongsToCurrentThread());

  if (IsClientSinkAvailable() || !rendering_ || !is_background_rendering_)
    return current_frame_;

  DCHECK(!last_background_render_.is_null());

  const base::TimeTicks now = tick_clock_->NowTicks();
  const base::TimeDelta interval = now - last_background_render_;

  // Cap updates to 250Hz which should be more than enough for everyone.
  if (interval < base::TimeDelta::FromMilliseconds(4))
    return current_frame_;

  // Update the interval based on the time between calls and call background
  // render which will give this information to the client.
  last_interval_ = interval;
  BackgroundRender();

  return current_frame_;
}

base::TimeDelta VideoFrameCompositor::GetCurrentFrameTimestamp() const {
  // When the VFC is stopped, |callback_| is cleared; this synchronously
  // prevents CallRender() from invoking ProcessNewFrame(), and so
  // |current_frame_| won't change again until after Start(). (Assuming that
  // PaintSingleFrame() is not also called while stopped.)
  if (!current_frame_)
    return base::TimeDelta();
  return current_frame_->timestamp();
}

void VideoFrameCompositor::SetOnNewProcessedFrameCallback(
    const OnNewProcessedFrameCB& cb) {
  DCHECK(task_runner_->BelongsToCurrentThread());
  new_processed_frame_cb_ = cb;
}

bool VideoFrameCompositor::ProcessNewFrame(
    const scoped_refptr<VideoFrame>& frame,
    bool repaint_duplicate_frame) {
  DCHECK(task_runner_->BelongsToCurrentThread());

  if (frame && current_frame_ && !repaint_duplicate_frame &&
      frame->unique_id() == current_frame_->unique_id()) {
    return false;
  }

  // Set the flag indicating that the current frame is unrendered, if we get a
  // subsequent PutCurrentFrame() call it will mark it as rendered.
  rendered_last_frame_ = false;

  current_frame_ = frame;

  if (!new_processed_frame_cb_.is_null())
    base::ResetAndReturn(&new_processed_frame_cb_).Run(base::TimeTicks::Now());

  return true;
}

void VideoFrameCompositor::BackgroundRender() {
  DCHECK(task_runner_->BelongsToCurrentThread());
  const base::TimeTicks now = tick_clock_->NowTicks();
  last_background_render_ = now;
  bool new_frame = CallRender(now, now + last_interval_, true);
  if (new_frame && IsClientSinkAvailable()) {
    if (!surface_layer_for_video_enabled_)
      client_->DidReceiveFrame();
    else
      submitter_->DidReceiveFrame();
  }
}

bool VideoFrameCompositor::CallRender(base::TimeTicks deadline_min,
                                      base::TimeTicks deadline_max,
                                      bool background_rendering) {
  DCHECK(task_runner_->BelongsToCurrentThread());
  base::AutoLock lock(callback_lock_);

  if (!callback_) {
    // Even if we no longer have a callback, return true if we have a frame
    // which |client_| hasn't seen before.
    return !rendered_last_frame_ && current_frame_;
  }

  DCHECK(rendering_);

  // If the previous frame was never rendered and we're not in background
  // rendering mode (nor have just exited it), let the client know.
  if (!rendered_last_frame_ && current_frame_ && !background_rendering &&
      !is_background_rendering_) {
    callback_->OnFrameDropped();
  }

  const bool new_frame = ProcessNewFrame(
      callback_->Render(deadline_min, deadline_max, background_rendering),
      false);

  // We may create a new frame here with background rendering, but the provider
  // has no way of knowing that a new frame had been processed, so keep track of
  // the new frame, and return true on the next call to |CallRender|.
  const bool had_new_background_frame = new_background_frame_;
  new_background_frame_ = background_rendering && new_frame;

  is_background_rendering_ = background_rendering;
  last_interval_ = deadline_max - deadline_min;

  // Restart the background rendering timer whether we're background rendering
  // or not; in either case we should wait for |kBackgroundRenderingTimeoutMs|.
  if (background_rendering_enabled_)
    background_rendering_timer_.Reset();
  return new_frame || had_new_background_frame;
}

}  // namespace media

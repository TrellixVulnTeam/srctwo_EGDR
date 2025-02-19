// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/media/session/media_session_uma_helper.h"

#include <utility>

#include "base/metrics/histogram_base.h"
#include "base/metrics/histogram_macros.h"
#include "base/time/default_tick_clock.h"

namespace content {

using HistogramBase = base::HistogramBase;

MediaSessionUmaHelper::MediaSessionUmaHelper()
    : clock_(new base::DefaultTickClock())
{}

MediaSessionUmaHelper::~MediaSessionUmaHelper()
{}

// static
void MediaSessionUmaHelper::RecordMediaSessionUserAction(
    MediaSessionUserAction action) {
  UMA_HISTOGRAM_ENUMERATION(
      "Media.Session.UserAction", static_cast<HistogramBase::Sample>(action),
      static_cast<HistogramBase::Sample>(MediaSessionUserAction::Count));
}

void MediaSessionUmaHelper::RecordSessionSuspended(
    MediaSessionSuspendedSource source) const {
  UMA_HISTOGRAM_ENUMERATION(
      "Media.Session.Suspended",
      static_cast<HistogramBase::Sample>(source),
      static_cast<HistogramBase::Sample>(MediaSessionSuspendedSource::Count));
}

void MediaSessionUmaHelper::RecordRequestAudioFocusResult(bool result) const {
  UMA_HISTOGRAM_BOOLEAN("Media.Session.RequestAudioFocusResult", result);
}

void MediaSessionUmaHelper::OnSessionActive() {
  current_active_time_ = clock_->NowTicks();
}

void MediaSessionUmaHelper::OnSessionSuspended() {
  if (current_active_time_.is_null())
    return;

  total_active_time_ += clock_->NowTicks() - current_active_time_;
  current_active_time_ = base::TimeTicks();
}

void MediaSessionUmaHelper::OnSessionInactive() {
  if (!current_active_time_.is_null()) {
    total_active_time_ += clock_->NowTicks() - current_active_time_;
    current_active_time_ = base::TimeTicks();
  }

  if (total_active_time_.is_zero())
    return;

  UMA_HISTOGRAM_LONG_TIMES("Media.Session.ActiveTime", total_active_time_);
  total_active_time_ = base::TimeDelta();
}

void MediaSessionUmaHelper::SetClockForTest(
    std::unique_ptr<base::TickClock> testing_clock) {
  clock_ = std::move(testing_clock);
}

}  // namespace content

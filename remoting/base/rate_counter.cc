// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/base/rate_counter.h"

#include "base/logging.h"

namespace remoting {

RateCounter::RateCounter(base::TimeDelta time_window)
    : time_window_(time_window), sum_(0) {
  DCHECK_GT(time_window, base::TimeDelta());
}

RateCounter::~RateCounter() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
}

void RateCounter::Record(int64_t value) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  base::TimeTicks now = tick_clock_->NowTicks();
  EvictOldDataPoints(now);
  sum_ += value;
  data_points_.push(std::make_pair(now, value));
}

double RateCounter::Rate() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  EvictOldDataPoints(tick_clock_->NowTicks());
  return sum_ / time_window_.InSecondsF();
}

void RateCounter::EvictOldDataPoints(base::TimeTicks now) {
  // Remove data points outside of the window.
  base::TimeTicks window_start = now - time_window_;

  while (!data_points_.empty()) {
    if (data_points_.front().first > window_start)
      break;

    sum_ -= data_points_.front().second;
    data_points_.pop();
  }
}

}  // namespace remoting

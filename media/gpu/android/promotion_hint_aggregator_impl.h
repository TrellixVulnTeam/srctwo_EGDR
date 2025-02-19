// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_GPU_ANDROID_PROMOTION_HINT_AGGREGATOR_IMPL_H_
#define MEDIA_GPU_ANDROID_PROMOTION_HINT_AGGREGATOR_IMPL_H_

#include <memory>

#include "base/bind.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/time/tick_clock.h"
#include "media/gpu/android/promotion_hint_aggregator.h"
#include "media/gpu/media_gpu_export.h"

namespace media {

// Receive lots of promotion hints, and aggregate them into a single signal.
class MEDIA_GPU_EXPORT PromotionHintAggregatorImpl
    : public PromotionHintAggregator {
 public:
  // |tick_clock| may be null, in which case we will use wall clock.  If it is
  // not null, then it must outlive |this|.  It is provided for tests.
  PromotionHintAggregatorImpl(base::TickClock* tick_clock = nullptr);
  ~PromotionHintAggregatorImpl() override;

  void NotifyPromotionHint(const Hint& hint) override;
  bool IsSafeToPromote() override;

 private:
  // Clock, which we might not own, that we'll use.
  base::TickClock* tick_clock_;

  // Will be non-null if we allocate our own clock.  Use |tick_clock| instead.
  std::unique_ptr<base::TickClock> clock_we_own_;

  // When did we receive the most recent "not promotable" frame?
  base::TimeTicks most_recent_unpromotable_;

  // When did we last receive an update?
  base::TimeTicks most_recent_update_;

  // Number of frames which were promotable in a row.
  int consecutive_promotable_frames_ = 0;

  base::WeakPtrFactory<PromotionHintAggregatorImpl> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(PromotionHintAggregatorImpl);
};

}  // namespace media

#endif  // MEDIA_GPU_ANDROID_PROMOTION_HINT_AGGREGATOR_IMPL_H_

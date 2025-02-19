// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_ANIMATION_ANIMATION_DELEGATE_H_
#define CC_ANIMATION_ANIMATION_DELEGATE_H_

#include "base/time/time.h"
#include "cc/animation/animation.h"
#include "cc/animation/animation_curve.h"

namespace cc {

class CC_ANIMATION_EXPORT AnimationDelegate {
 public:
  virtual void NotifyAnimationStarted(base::TimeTicks monotonic_time,
                                      int target_property,
                                      int group) = 0;
  virtual void NotifyAnimationFinished(base::TimeTicks monotonic_time,
                                       int target_property,
                                       int group) = 0;

  virtual void NotifyAnimationAborted(base::TimeTicks monotonic_time,
                                      int target_property,
                                      int group) = 0;

  virtual void NotifyAnimationTakeover(
      base::TimeTicks monotonic_time,
      int target_property,
      base::TimeTicks animation_start_time,
      std::unique_ptr<AnimationCurve> curve) = 0;

 protected:
  virtual ~AnimationDelegate() {}
};

}  // namespace cc

#endif  // CC_ANIMATION_ANIMATION_DELEGATE_H_

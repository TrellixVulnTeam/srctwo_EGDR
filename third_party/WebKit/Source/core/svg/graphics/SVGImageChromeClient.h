/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SVGImageChromeClient_h
#define SVGImageChromeClient_h

#include "base/gtest_prod_util.h"
#include "core/CoreExport.h"
#include "core/loader/EmptyClients.h"
#include "platform/Timer.h"
#include <memory>

namespace blink {

class SVGImage;

class CORE_EXPORT SVGImageChromeClient final : public EmptyChromeClient {
 public:
  static SVGImageChromeClient* Create(SVGImage*);

  bool IsSVGImageChromeClient() const override;

  SVGImage* GetImage() const { return image_; }

  void SuspendAnimation();
  void ResumeAnimation();
  bool IsSuspended() const { return timeline_state_ >= kSuspended; }

 private:
  explicit SVGImageChromeClient(SVGImage*);

  void ChromeDestroyed() override;
  void InvalidateRect(const IntRect&) override;
  void ScheduleAnimation(const PlatformFrameView*) override;

  void SetTimer(std::unique_ptr<TimerBase>);
  void AnimationTimerFired(TimerBase*);

  SVGImage* image_;
  std::unique_ptr<TimerBase> animation_timer_;
  enum {
    kRunning,
    kSuspended,
    kSuspendedWithAnimationPending,
  } timeline_state_;

  FRIEND_TEST_ALL_PREFIXES(SVGImageTest, TimelineSuspendAndResume);
  FRIEND_TEST_ALL_PREFIXES(SVGImageTest, ResetAnimation);
};

DEFINE_TYPE_CASTS(SVGImageChromeClient,
                  ChromeClient,
                  client,
                  client->IsSVGImageChromeClient(),
                  client.IsSVGImageChromeClient());

}  // namespace blink

#endif  // SVGImageChromeClient_h

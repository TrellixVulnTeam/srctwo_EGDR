// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ScrollAnimatorCompositorCoordinator_h
#define ScrollAnimatorCompositorCoordinator_h

#include <memory>
#include "base/gtest_prod_util.h"
#include "cc/animation/animation_curve.h"
#include "cc/animation/scroll_offset_animations.h"
#include "platform/PlatformExport.h"
#include "platform/animation/CompositorAnimationDelegate.h"
#include "platform/animation/CompositorAnimationPlayerClient.h"
#include "platform/graphics/CompositorElementId.h"
#include "platform/heap/Handle.h"
#include "platform/scroll/ScrollTypes.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/Noncopyable.h"

namespace blink {

class ScrollableArea;
class CompositorAnimation;
class CompositorAnimationPlayer;
class CompositorAnimationTimeline;

// ScrollAnimatorCompositorCoordinator is the common base class of user scroll
// animators and programmatic scroll animators, and holds logic related to
// scheduling and updating scroll animations running on the compositor.
//
// See ScrollAnimator.h for more information about scroll animations.

class PLATFORM_EXPORT ScrollAnimatorCompositorCoordinator
    : public GarbageCollectedFinalized<ScrollAnimatorCompositorCoordinator>,
      private CompositorAnimationPlayerClient,
      CompositorAnimationDelegate {
  WTF_MAKE_NONCOPYABLE(ScrollAnimatorCompositorCoordinator);
  USING_PRE_FINALIZER(ScrollAnimatorCompositorCoordinator, Dispose);

 public:
  enum class RunState {
    // No animation.
    kIdle,

    // Waiting to send an animation to the compositor. There might also
    // already be another animation running on the compositor that will need
    // to be canceled first.
    kWaitingToSendToCompositor,

    // Running an animation on the compositor.
    kRunningOnCompositor,

    // Running an animation on the compositor but needs update.
    kRunningOnCompositorButNeedsUpdate,

    // Running an animation on the main thread.
    kRunningOnMainThread,

    // Waiting to cancel the animation currently running on the compositor.
    // There is no pending animation to replace the canceled animation.
    kWaitingToCancelOnCompositor,

    // Finished an animation that was running on the main thread or the
    // compositor thread. When in this state, post animation cleanup can
    // be performed.
    kPostAnimationCleanup,

    // Running an animation on the compositor but need to continue it
    // on the main thread. This could happen if a main thread scrolling
    // reason is added while animating the scroll offset.
    kRunningOnCompositorButNeedsTakeover,

    // Waiting to cancel the animation currently running on the compositor
    // while another animation is requested. In this case, the currently
    // running animation is aborted and an animation to the new target
    // from the current offset is started.
    kWaitingToCancelOnCompositorButNewScroll,

    // Running an animation on the compositor but an adjustment to the
    // scroll offset was made on the main thread and the animation must
    // be updated.
    kRunningOnCompositorButNeedsAdjustment,
  };

  virtual ~ScrollAnimatorCompositorCoordinator();

  bool HasAnimationThatRequiresService() const;
  void Dispose();
  String RunStateAsText() const;

  virtual bool HasRunningAnimation() const { return false; }

  virtual void ResetAnimationState();
  virtual void CancelAnimation();
  // Aborts the currently running scroll offset animation on the compositor
  // and continues it on the main thread. This should only be called when in
  // DocumentLifecycle::LifecycleState::CompositingClean state.
  virtual void TakeOverCompositorAnimation();
  // Updates the scroll offset of the animator's ScrollableArea by
  // adjustment and update the target of an ongoing scroll offset animation.
  virtual void AdjustAnimationAndSetScrollOffset(const ScrollOffset&,
                                                 ScrollType);
  virtual void UpdateCompositorAnimations();

  virtual ScrollableArea* GetScrollableArea() const = 0;
  virtual void TickAnimation(double monotonic_time) = 0;
  virtual void NotifyCompositorAnimationFinished(int group_id) = 0;
  virtual void NotifyCompositorAnimationAborted(int group_id) = 0;
  virtual void LayerForCompositedScrollingDidChange(
      CompositorAnimationTimeline*) = 0;

  RunState RunStateForTesting() { return run_state_; }

  DEFINE_INLINE_VIRTUAL_TRACE() {}

 protected:
  explicit ScrollAnimatorCompositorCoordinator();

  void ScrollOffsetChanged(const ScrollOffset&, ScrollType);

  void AdjustImplOnlyScrollOffsetAnimation(const IntSize& adjustment);
  IntSize ImplOnlyAnimationAdjustmentForTesting() {
    return impl_only_animation_adjustment_;
  }

  void ResetAnimationIds();
  bool AddAnimation(std::unique_ptr<CompositorAnimation>);
  void RemoveAnimation();
  virtual void AbortAnimation();

  // "offset" in the cc scrolling code is analagous to "position" in the blink
  // scrolling code:
  // they both represent the distance from the top-left of the overflow rect to
  // the top-left
  // of the viewport.  In blink, "offset" refers to the distance of the viewport
  // from the
  // beginning of flow of the contents.  In left-to-right flows, blink "offset"
  // and "position" are
  // equivalent, but in right-to-left flows (including direction:rtl,
  // writing-mode:vertical-rl,
  // and flex-direction:row-reverse), they aren't.  See core/layout/README.md
  // for more info.
  FloatPoint CompositorOffsetFromBlinkOffset(ScrollOffset);
  ScrollOffset BlinkOffsetFromCompositorOffset(FloatPoint);

  void CompositorAnimationFinished(int group_id);
  // Returns true if the compositor player was attached to a new layer.
  bool ReattachCompositorPlayerIfNeeded(CompositorAnimationTimeline*);

  // CompositorAnimationDelegate implementation.
  void NotifyAnimationStarted(double monotonic_time, int group) override;
  void NotifyAnimationFinished(double monotonic_time, int group) override;
  void NotifyAnimationAborted(double monotonic_time, int group) override;
  void NotifyAnimationTakeover(double monotonic_time,
                               double animation_start_time,
                               std::unique_ptr<cc::AnimationCurve>) override {}

  // CompositorAnimationPlayerClient implementation.
  CompositorAnimationPlayer* CompositorPlayer() const override;

  friend class Internals;
  // TODO(ymalik): Tests are added as friends to access m_RunState. Use the
  // runStateForTesting accessor instead.
  FRIEND_TEST_ALL_PREFIXES(ScrollAnimatorTest, MainThreadStates);
  FRIEND_TEST_ALL_PREFIXES(ScrollAnimatorTest, AnimatedScrollTakeover);
  FRIEND_TEST_ALL_PREFIXES(ScrollAnimatorTest, CancellingAnimationResetsState);
  FRIEND_TEST_ALL_PREFIXES(ScrollAnimatorTest, CancellingCompositorAnimation);
  FRIEND_TEST_ALL_PREFIXES(ScrollAnimatorTest, ImplOnlyAnimationUpdatesCleared);

  std::unique_ptr<CompositorAnimationPlayer> compositor_player_;
  // The element id to which the compositor animation is attached when
  // the animation is present.
  CompositorElementId element_id_;
  RunState run_state_;
  int compositor_animation_id_;
  int compositor_animation_group_id_;

  // An adjustment to the scroll offset on the main thread that may affect
  // impl-only scroll offset animations.
  IntSize impl_only_animation_adjustment_;

  // If set to true, sends a cc::ScrollOffsetAnimationUpdate to cc which will
  // abort the impl-only scroll offset animation and continue it on main
  // thread.
  bool impl_only_animation_takeover_;

 private:
  CompositorElementId GetScrollElementId() const;
  bool HasImplOnlyAnimationUpdate() const;
  void UpdateImplOnlyCompositorAnimations();
  // Accesses compositing state and should only be called when in or after
  // DocumentLifecycle::LifecycleState::CompositingClean.
  void TakeOverImplOnlyScrollOffsetAnimation();
};

}  // namespace blink

#endif  // ScrollAnimatorCompositorCoordinator_h

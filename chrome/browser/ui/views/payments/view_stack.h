// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_PAYMENTS_VIEW_STACK_H_
#define CHROME_BROWSER_UI_VIEWS_PAYMENTS_VIEW_STACK_H_

#include <vector>

#include "ui/views/view.h"
#include "ui/views/animation/bounds_animator.h"
#include "ui/views/animation/bounds_animator_observer.h"

namespace payments {
class PaymentRequestBrowserTestBase;
}  // namespace payments

// This view represents a stack of views that slide in over one another from
// left to right. It manages the animation and lifetime of views that are
// pushed and popped on it. To use this class, add it to a view hierarchy, and
// call Push/Pop to animate views in and out.
class ViewStack : public views::BoundsAnimatorObserver,
                  public views::View {
 public:
  ViewStack();
  ~ViewStack() override;

  // Adds a view to the stack and starts animating it in from the right. This
  // takes ownership of the view and calls set_owned_by_client() on it.
  // If |animate| is false, the view will simply be added to the hierarchy
  // without the sliding animation.
  void Push(std::unique_ptr<views::View> state, bool animate);

  // Removes a view from the stack, animates it out of view, and makes sure
  // it's properly deleted after the animation.
  void Pop();

  // Removes |n| views from the stack but only animates the topmost one. The end
  // result is an animation from the top-most view to the destination view.
  void PopMany(int n);

  size_t size() const;

  // views::View:
  // The children of this view must not be able to process events when the views
  // are being animated so this returns false when an animation is in progress.
  bool CanProcessEventsWithinSubtree() const override;
  void Layout() override;
  void RequestFocus() override;

  // Returns the top state of the stack.
  views::View* top() { return stack_.back().get(); }

 private:
  FRIEND_TEST_ALL_PREFIXES(
      ViewStackTest, TestPopStateRemovesChildViewAndCleansUpState);
  FRIEND_TEST_ALL_PREFIXES(ViewStackTest, TestDeletingViewCleansUpState);
  FRIEND_TEST_ALL_PREFIXES(ViewStackTest, TestInitialStateAddedAsChildView);
  FRIEND_TEST_ALL_PREFIXES(ViewStackTest, TestPushStateAddsViewToChildren);
  FRIEND_TEST_ALL_PREFIXES(ViewStackTest, TestLayoutUpdatesAnimations);
  friend class ViewStackTest;
  friend class payments::PaymentRequestBrowserTestBase;

  // Marks all views, except the topmost, as invisible.
  void HideCoveredViews();

  void UpdateAnimatorBounds(
      views::BoundsAnimator* animator, const gfx::Rect& target);

  // views::BoundsAnimatorObserver:
  void OnBoundsAnimatorProgressed(views::BoundsAnimator* animator) override {}
  void OnBoundsAnimatorDone(views::BoundsAnimator* animator) override;

  std::unique_ptr<views::BoundsAnimator> slide_in_animator_;
  std::unique_ptr<views::BoundsAnimator> slide_out_animator_;

  // Should be the last member, because views need to be destroyed before other
  // members, and members are destroyed in reverse order of their creation.
  std::vector<std::unique_ptr<views::View>> stack_;

  DISALLOW_COPY_AND_ASSIGN(ViewStack);
};

#endif  // CHROME_BROWSER_UI_VIEWS_PAYMENTS_VIEW_STACK_H_

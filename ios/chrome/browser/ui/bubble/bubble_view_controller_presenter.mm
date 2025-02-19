// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/bubble/bubble_view_controller_presenter.h"

#import "base/ios/block_types.h"
#include "base/logging.h"
#include "ios/chrome/browser/ui/bubble/bubble_util.h"
#import "ios/chrome/browser/ui/bubble/bubble_view_controller.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace {

// How long, in seconds, the bubble is visible on the screen.
const NSTimeInterval kBubbleVisibilityDuration = 3.0;
// How long, in seconds, the user should be considered engaged with the bubble
// after the bubble first becomes visible.
const NSTimeInterval kBubbleEngagementDuration = 30.0;

}  // namespace

@interface BubbleViewControllerPresenter ()<UIGestureRecognizerDelegate>

// Redeclared as readwrite so the value can be changed internally.
@property(nonatomic, assign, readwrite, getter=isUserEngaged) BOOL userEngaged;
// The underlying BubbleViewController managed by this object.
// |bubbleViewController| manages the BubbleView instance.
@property(nonatomic, strong) BubbleViewController* bubbleViewController;
// The tap gesture recognizer intercepting tap gestures occurring inside the
// bubble view. Taps inside must be differentiated from taps outside to track
// UMA metrics.
@property(nonatomic, strong) UITapGestureRecognizer* insideBubbleTapRecognizer;
// The tap gesture recognizer intercepting tap gestures occurring outside the
// bubble view. Does not prevent interactions with elements being tapped on.
// For example, tapping on a button both dismisses the bubble and triggers the
// button's action.
@property(nonatomic, strong) UITapGestureRecognizer* outsideBubbleTapRecognizer;
// The timer used to dismiss the bubble after a certain length of time. The
// bubble is dismissed automatically if the user does not dismiss it manually.
// If the user dismisses it manually, this timer is invalidated. The timer
// maintains a strong reference to the presenter, so it must be retained weakly
// to prevent a retain cycle. The run loop retains a strong reference to the
// timer so it is not deallocated until it is invalidated.
@property(nonatomic, weak) NSTimer* bubbleDismissalTimer;
// The timer used to reset the user's engagement. The user is considered
// engaged with the bubble while it is visible and for a certain duration after
// it disappears. The timer maintains a strong reference to the presenter, so it
// must be retained weakly to prevent a retain cycle. The run loop retains a
// strong reference to the timer so it is not deallocated until it is
// invalidated.
@property(nonatomic, weak) NSTimer* engagementTimer;
// The direction the underlying BubbleView's arrow is pointing.
@property(nonatomic, assign) BubbleArrowDirection arrowDirection;
// The alignment of the underlying BubbleView's arrow.
@property(nonatomic, assign) BubbleAlignment alignment;
// The block invoked when the bubble is dismissed (both via timer and via tap).
// Is optional.
@property(nonatomic, strong) ProceduralBlock dismissalCallback;

@end

@implementation BubbleViewControllerPresenter

@synthesize bubbleViewController = _bubbleViewController;
@synthesize insideBubbleTapRecognizer = _insideBubbleTapRecognizer;
@synthesize outsideBubbleTapRecognizer = _outsideBubbleTapRecognizer;
@synthesize bubbleDismissalTimer = _bubbleDismissalTimer;
@synthesize engagementTimer = _engagementTimer;
@synthesize userEngaged = _userEngaged;
@synthesize triggerFollowUpAction = _triggerFollowUpAction;
@synthesize arrowDirection = _arrowDirection;
@synthesize alignment = _alignment;
@synthesize dismissalCallback = _dismissalCallback;

- (instancetype)initWithText:(NSString*)text
              arrowDirection:(BubbleArrowDirection)arrowDirection
                   alignment:(BubbleAlignment)alignment
           dismissalCallback:(ProceduralBlock)dismissalCallback {
  self = [super init];
  if (self) {
    _bubbleViewController =
        [[BubbleViewController alloc] initWithText:text
                                    arrowDirection:arrowDirection
                                         alignment:alignment];
    _outsideBubbleTapRecognizer = [[UITapGestureRecognizer alloc]
        initWithTarget:self
                action:@selector(tapOutsideBubbleRecognized:)];
    _outsideBubbleTapRecognizer.delegate = self;
    _outsideBubbleTapRecognizer.cancelsTouchesInView = NO;
    _insideBubbleTapRecognizer = [[UITapGestureRecognizer alloc]
        initWithTarget:self
                action:@selector(tapInsideBubbleRecognized:)];
    _insideBubbleTapRecognizer.delegate = self;
    _insideBubbleTapRecognizer.cancelsTouchesInView = NO;
    _userEngaged = NO;
    _triggerFollowUpAction = NO;
    _arrowDirection = arrowDirection;
    _alignment = alignment;
    _dismissalCallback = dismissalCallback;
    // The timers are initialized when the bubble is presented, not during
    // initialization. Because the user might not present the bubble immediately
    // after initialization, the timers cannot be started until the bubble
    // appears on screen.
  }
  return self;
}

- (void)presentInViewController:(UIViewController*)parentViewController
                           view:(UIView*)parentView
                    anchorPoint:(CGPoint)anchorPoint {
  [parentViewController addChildViewController:self.bubbleViewController];

  CGPoint anchorPointInParent =
      [parentView.window convertPoint:anchorPoint toView:parentView];
  self.bubbleViewController.view.frame =
      [self frameForBubbleInRect:parentView.bounds
                   atAnchorPoint:anchorPointInParent];
  [parentView addSubview:self.bubbleViewController.view];
  [self.bubbleViewController animateContentIn];

  [self.bubbleViewController.view
      addGestureRecognizer:self.insideBubbleTapRecognizer];
  [parentView addGestureRecognizer:self.outsideBubbleTapRecognizer];

  self.bubbleDismissalTimer = [NSTimer
      scheduledTimerWithTimeInterval:kBubbleVisibilityDuration
                              target:self
                            selector:@selector(bubbleDismissalTimerFired:)
                            userInfo:nil
                             repeats:NO];

  self.userEngaged = YES;
  self.triggerFollowUpAction = YES;
  self.engagementTimer =
      [NSTimer scheduledTimerWithTimeInterval:kBubbleEngagementDuration
                                       target:self
                                     selector:@selector(engagementTimerFired:)
                                     userInfo:nil
                                      repeats:NO];
}

- (void)dismissAnimated:(BOOL)animated {
  // Because this object must stay in memory to handle the |userEngaged|
  // property correctly, it is possible for |dismissAnimated| to be called
  // multiple times. However, only the first call should have any effect.
  if (!self.bubbleViewController.parentViewController) {
    return;
  }

  [self.bubbleDismissalTimer invalidate];
  self.bubbleDismissalTimer = nil;
  [self.insideBubbleTapRecognizer.view
      removeGestureRecognizer:self.insideBubbleTapRecognizer];
  [self.outsideBubbleTapRecognizer.view
      removeGestureRecognizer:self.outsideBubbleTapRecognizer];
  [self.bubbleViewController dismissAnimated:animated];
  [self.bubbleViewController willMoveToParentViewController:nil];
  [self.bubbleViewController removeFromParentViewController];

  if (self.dismissalCallback) {
    self.dismissalCallback();
  }
}

- (void)dealloc {
  [self.bubbleDismissalTimer invalidate];
  self.bubbleDismissalTimer = nil;
  [self.engagementTimer invalidate];
  self.engagementTimer = nil;
  [self.insideBubbleTapRecognizer.view
      removeGestureRecognizer:self.insideBubbleTapRecognizer];
  [self.outsideBubbleTapRecognizer.view
      removeGestureRecognizer:self.outsideBubbleTapRecognizer];
}

#pragma mark - UIGestureRecognizerDelegate

- (BOOL)gestureRecognizer:(UIGestureRecognizer*)gestureRecognizer
    shouldRecognizeSimultaneouslyWithGestureRecognizer:
        (UIGestureRecognizer*)otherGestureRecognizer {
  // Because the outside tap recognizer is potentially in the responder chain,
  // this prevents both the inside and outside gesture recognizers from
  // triggering at once when tapping inside the bubble.
  return gestureRecognizer != self.insideBubbleTapRecognizer &&
         otherGestureRecognizer != self.insideBubbleTapRecognizer;
}

#pragma mark - Private

// Invoked by tapping inside the bubble. Dismisses the bubble.
- (void)tapInsideBubbleRecognized:(id)sender {
  [self dismissAnimated:YES];
}

// Invoked by tapping outside the bubble. Dismisses the bubble.
- (void)tapOutsideBubbleRecognized:(id)sender {
  [self dismissAnimated:YES];
}

// Automatically dismisses the bubble view when |bubbleDismissalTimer| fires.
- (void)bubbleDismissalTimerFired:(id)sender {
  [self dismissAnimated:YES];
}

// Marks the user as not engaged when |engagementTimer| fires.
- (void)engagementTimerFired:(id)sender {
  self.userEngaged = NO;
  self.triggerFollowUpAction = NO;
  self.engagementTimer = nil;
}

// Calculates the frame of the BubbleView. |rect| is the frame of the bubble's
// superview. |anchorPoint| is the anchor point of the bubble. |anchorPoint|
// and |rect| must be in the same coordinates.
- (CGRect)frameForBubbleInRect:(CGRect)rect atAnchorPoint:(CGPoint)anchorPoint {
  CGSize maxBubbleSize = bubble_util::BubbleMaxSize(
      anchorPoint, self.arrowDirection, self.alignment, rect.size);
  CGSize bubbleSize =
      [self.bubbleViewController.view sizeThatFits:maxBubbleSize];
  // If |bubbleSize| does not fit in |maxBubbleSize|, the bubble will be
  // partially off screen and not look good. This is most likely a result of
  // an incorrect value for |alignment| (such as a trailing aligned bubble
  // anchored to an element on the leading edge of the screen).
  DCHECK(bubbleSize.width <= maxBubbleSize.width);
  DCHECK(bubbleSize.height <= maxBubbleSize.height);
  CGRect bubbleFrame =
      bubble_util::BubbleFrame(anchorPoint, bubbleSize, self.arrowDirection,
                               self.alignment, CGRectGetWidth(rect));
  return bubbleFrame;
}

@end

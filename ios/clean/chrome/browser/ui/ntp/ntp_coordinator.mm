// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/clean/chrome/browser/ui/ntp/ntp_coordinator.h"

#include "base/logging.h"
#import "ios/chrome/browser/ui/broadcaster/chrome_broadcaster.h"
#import "ios/chrome/browser/ui/browser_list/browser.h"
#import "ios/chrome/browser/ui/commands/command_dispatcher.h"
#import "ios/chrome/browser/ui/content_suggestions/ntp_home_constant.h"
#import "ios/chrome/browser/ui/coordinators/browser_coordinator+internal.h"
#include "ios/chrome/browser/ui/ui_util.h"
#import "ios/clean/chrome/browser/ui/bookmarks/bookmarks_coordinator.h"
#import "ios/clean/chrome/browser/ui/commands/ntp_commands.h"
#import "ios/clean/chrome/browser/ui/ntp/ntp_home_coordinator.h"
#import "ios/clean/chrome/browser/ui/ntp/ntp_mediator.h"
#import "ios/clean/chrome/browser/ui/ntp/ntp_view_controller.h"
#import "ios/clean/chrome/browser/ui/recent_tabs/recent_tabs_coordinator.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

@interface NTPCoordinator ()<NTPCommands>
@property(nonatomic, strong) NTPMediator* mediator;
@property(nonatomic, strong) NTPViewController* viewController;
@property(nonatomic, strong) NTPHomeCoordinator* homeCoordinator;
@property(nonatomic, strong) BookmarksCoordinator* bookmarksCoordinator;
@property(nonatomic, strong) RecentTabsCoordinator* recentTabsCoordinator;
@end

@implementation NTPCoordinator
@synthesize mediator = _mediator;
@synthesize viewController = _viewController;
@synthesize homeCoordinator = _homeCoordinator;
@synthesize bookmarksCoordinator = _bookmarksCoordinator;
@synthesize recentTabsCoordinator = _recentTabsCoordinator;

- (void)start {
  if (self.started)
    return;

  self.viewController = [[NTPViewController alloc] init];
  self.mediator = [[NTPMediator alloc] initWithConsumer:self.viewController];

  CommandDispatcher* dispatcher = self.browser->dispatcher();
  // NTPCommands
  [dispatcher startDispatchingToTarget:self forProtocol:@protocol(NTPCommands)];
  self.viewController.dispatcher = static_cast<id>(self.browser->dispatcher());
  [self.browser->broadcaster()
      broadcastValue:@"selectedNTPPanel"
            ofObject:self.viewController
            selector:@selector(broadcastSelectedNTPPanel:)];
  [super start];
}

- (void)stop {
  [super stop];
  [self.browser->broadcaster()
      stopBroadcastingForSelector:@selector(broadcastSelectedNTPPanel:)];
  [self.browser->dispatcher() stopDispatchingToTarget:self];
}

- (void)childCoordinatorDidStart:(BrowserCoordinator*)coordinator {
  if ([coordinator isKindOfClass:[NTPHomeCoordinator class]]) {
    self.viewController.homeViewController = coordinator.viewController;

  } else if (coordinator == self.bookmarksCoordinator) {
    if (self.bookmarksCoordinator.mode == CONTAINED) {
      self.viewController.bookmarksViewController = coordinator.viewController;
    } else {
      coordinator.viewController.modalPresentationStyle =
          UIModalPresentationFormSheet;
      [self.viewController presentViewController:coordinator.viewController
                                        animated:YES
                                      completion:nil];
    }

  } else if (coordinator == self.recentTabsCoordinator) {
    if (self.recentTabsCoordinator.mode == CONTAINED) {
      self.viewController.recentTabsViewController = coordinator.viewController;
    } else {
      coordinator.viewController.modalPresentationStyle =
          UIModalPresentationFormSheet;
      coordinator.viewController.modalPresentationCapturesStatusBarAppearance =
          YES;
      [self.viewController presentViewController:coordinator.viewController
                                        animated:YES
                                      completion:nil];
    }
  }
}

- (void)childCoordinatorWillStop:(BrowserCoordinator*)childCoordinator {
  if ([childCoordinator isKindOfClass:[BookmarksCoordinator class]] &&
      !IsIPadIdiom()) {
    [childCoordinator.viewController.presentingViewController
        dismissViewControllerAnimated:YES
                           completion:nil];
  } else if ([childCoordinator isKindOfClass:[RecentTabsCoordinator class]] &&
             !IsIPadIdiom()) {
    [childCoordinator.viewController.presentingViewController
        dismissViewControllerAnimated:YES
                           completion:nil];
  }
}

#pragma mark - NTPCommands

- (void)showNTPHomePanel {
  NTPHomeCoordinator* panelCoordinator = [[NTPHomeCoordinator alloc] init];
  [self addChildCoordinator:panelCoordinator];
  [panelCoordinator start];
}

- (void)showNTPBookmarksPanel {
  if (!self.bookmarksCoordinator) {
    self.bookmarksCoordinator = [[BookmarksCoordinator alloc] init];
    self.bookmarksCoordinator.mode = IsIPadIdiom() ? CONTAINED : PRESENTED;
    [self addChildCoordinator:self.bookmarksCoordinator];
  }
  [self.bookmarksCoordinator start];
}

- (void)showNTPRecentTabsPanel {
  if (!self.recentTabsCoordinator) {
    self.recentTabsCoordinator = [[RecentTabsCoordinator alloc] init];
    self.recentTabsCoordinator.mode = IsIPadIdiom() ? CONTAINED : PRESENTED;
    [self addChildCoordinator:self.recentTabsCoordinator];
  }
  [self.recentTabsCoordinator start];
}

@end

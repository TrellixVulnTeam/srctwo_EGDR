// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/clean/chrome/browser/ui/tab_grid/tab_grid_coordinator.h"

#include <memory>

#include "base/mac/foundation_util.h"
#include "base/memory/ptr_util.h"
#include "base/strings/sys_string_conversions.h"
#include "ios/chrome/browser/browser_state/chrome_browser_state.h"
#import "ios/chrome/browser/snapshots/snapshot_cache_factory.h"
#import "ios/chrome/browser/ui/browser_list/browser.h"
#import "ios/chrome/browser/ui/commands/command_dispatcher.h"
#import "ios/chrome/browser/ui/coordinators/browser_coordinator+internal.h"
#import "ios/chrome/browser/ui/tools_menu/tools_menu_configuration.h"
#import "ios/chrome/browser/web_state_list/web_state_list.h"
#import "ios/chrome/browser/web_state_list/web_state_opener.h"
#import "ios/clean/chrome/browser/ui/commands/context_menu_commands.h"
#import "ios/clean/chrome/browser/ui/commands/settings_commands.h"
#import "ios/clean/chrome/browser/ui/commands/tab_grid_commands.h"
#import "ios/clean/chrome/browser/ui/commands/tools_menu_commands.h"
#import "ios/clean/chrome/browser/ui/dialogs/context_menu/context_menu_dialog_request.h"
#import "ios/clean/chrome/browser/ui/overlays/overlay_service.h"
#import "ios/clean/chrome/browser/ui/overlays/overlay_service_factory.h"
#import "ios/clean/chrome/browser/ui/overlays/overlay_service_observer_bridge.h"
#import "ios/clean/chrome/browser/ui/settings/settings_coordinator.h"
#import "ios/clean/chrome/browser/ui/tab/tab_coordinator.h"
#include "ios/clean/chrome/browser/ui/tab/tab_features.h"
#import "ios/clean/chrome/browser/ui/tab/tab_strip_tab_coordinator.h"
#import "ios/clean/chrome/browser/ui/tab_grid/tab_grid_mediator.h"
#import "ios/clean/chrome/browser/ui/tab_grid/tab_grid_view_controller.h"
#import "ios/clean/chrome/browser/ui/tools/tools_coordinator.h"
#import "ios/web/public/navigation_manager.h"
#include "ios/web/public/web_state/web_state.h"
#import "net/base/mac/url_conversions.h"
#include "ui/base/page_transition_types.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

@interface TabGridCoordinator ()<ContextMenuCommands,
                                 OverlayServiceObserving,
                                 SettingsCommands,
                                 TabGridCommands,
                                 ToolsMenuCommands> {
  // Bridge that handles forwarding OverlayServiceObserver events.
  std::unique_ptr<OverlayServiceObserverBridge> _overlayObserverBridge;
}

@property(nonatomic, strong) TabGridViewController* viewController;
@property(nonatomic, weak) SettingsCoordinator* settingsCoordinator;
@property(nonatomic, weak) ToolsCoordinator* toolsMenuCoordinator;
@property(nonatomic, weak) TabCoordinator* activeTabCoordinator;
@property(nonatomic, readonly) WebStateList& webStateList;
@property(nonatomic, strong) TabGridMediator* mediator;
@property(nonatomic, readonly) SnapshotCache* snapshotCache;

@end

@implementation TabGridCoordinator
@synthesize viewController = _viewController;
@synthesize settingsCoordinator = _settingsCoordinator;
@synthesize toolsMenuCoordinator = _toolsMenuCoordinator;
@synthesize activeTabCoordinator = _activeTabCoordinator;
@synthesize mediator = _mediator;

- (instancetype)init {
  if ((self = [super init])) {
    _overlayObserverBridge =
        base::MakeUnique<OverlayServiceObserverBridge>(self);
  }
  return self;
}

#pragma mark - Properties

- (WebStateList&)webStateList {
  return self.browser->web_state_list();
}

- (SnapshotCache*)snapshotCache {
  return SnapshotCacheFactory::GetForBrowserState(
      self.browser->browser_state());
}

#pragma mark - BrowserCoordinator

- (void)start {
  if (self.started)
    return;
  self.mediator = [[TabGridMediator alloc] init];
  self.mediator.webStateList = &self.webStateList;

  [self registerForContextMenuCommands];
  [self registerForSettingsCommands];
  [self registerForTabGridCommands];
  [self registerForToolsMenuCommands];

  self.viewController = [[TabGridViewController alloc] init];
  self.viewController.dispatcher = static_cast<id>(self.browser->dispatcher());
  self.viewController.snapshotCache = self.snapshotCache;

  self.mediator.consumer = self.viewController;

  OverlayServiceFactory::GetInstance()
      ->GetForBrowserState(self.browser->browser_state())
      ->AddObserver(_overlayObserverBridge.get());

  [super start];
}

- (void)stop {
  [super stop];
  [self.browser->dispatcher() stopDispatchingToTarget:self];
  [self.mediator disconnect];

  OverlayServiceFactory::GetInstance()
      ->GetForBrowserState(self.browser->browser_state())
      ->RemoveObserver(_overlayObserverBridge.get());

  // PLACEHOLDER: Remove child coordinators here for now. This might be handled
  // differently later on.
  for (BrowserCoordinator* child in self.children) {
    [self removeChildCoordinator:child];
  }
}

- (void)childCoordinatorDidStart:(BrowserCoordinator*)childCoordinator {
  DCHECK([childCoordinator isKindOfClass:[SettingsCoordinator class]] ||
         [childCoordinator isKindOfClass:[TabCoordinator class]] ||
         [childCoordinator isKindOfClass:[ToolsCoordinator class]]);
  [self.viewController presentViewController:childCoordinator.viewController
                                    animated:YES
                                  completion:nil];
}

- (void)childCoordinatorWillStop:(BrowserCoordinator*)childCoordinator {
  DCHECK([childCoordinator isKindOfClass:[SettingsCoordinator class]] ||
         [childCoordinator isKindOfClass:[TabCoordinator class]] ||
         [childCoordinator isKindOfClass:[ToolsCoordinator class]]);
  [childCoordinator.viewController.presentingViewController
      dismissViewControllerAnimated:YES
                         completion:nil];
}

#pragma mark - ContextMenuCommands

- (void)openContextMenuLinkInNewTab:(ContextMenuDialogRequest*)request {
  [self createAndShowNewTabInTabGrid];
  [self openURL:net::NSURLWithGURL(request.linkURL)];
}

- (void)openContextMenuImageInNewTab:(ContextMenuDialogRequest*)request {
  [self createAndShowNewTabInTabGrid];
  [self openURL:net::NSURLWithGURL(request.imageURL)];
}

#pragma mark - OverlayServiceObserving

- (void)overlayService:(OverlayService*)overlayService
    willShowOverlayForWebState:(web::WebState*)webState
                     inBrowser:(Browser*)browser {
  // If |webState| is specified, activate it in the WebStateList and ensure that
  // its content area is visible.
  WebStateList& webStateList = self.browser->web_state_list();
  if (webState && webStateList.GetActiveWebState() != webState) {
    int newActiveIndex = webStateList.GetIndexOfWebState(webState);
    DCHECK_NE(newActiveIndex, WebStateList::kInvalidIndex);
    webStateList.ActivateWebStateAt(newActiveIndex);
    [self showTabGridTabAtIndex:newActiveIndex];
  }
}

#pragma mark - SettingsCommands

- (void)showSettings {
  CommandDispatcher* dispatcher = self.browser->dispatcher();
  [dispatcher startDispatchingToTarget:self
                           forSelector:@selector(closeSettings)];
  SettingsCoordinator* settingsCoordinator = [[SettingsCoordinator alloc] init];
  [self addOverlayCoordinator:settingsCoordinator];
  self.settingsCoordinator = settingsCoordinator;
  [settingsCoordinator start];
}

- (void)closeSettings {
  CommandDispatcher* dispatcher = self.browser->dispatcher();
  [dispatcher stopDispatchingForSelector:@selector(closeSettings)];
  [self.settingsCoordinator stop];
  [self.settingsCoordinator.parentCoordinator
      removeChildCoordinator:self.settingsCoordinator];
  // self.settingsCoordinator should be presumed to be nil after this point.
}

#pragma mark - TabGridCommands

- (void)showTabGridTabAtIndex:(int)index {
  self.webStateList.ActivateWebStateAt(index);
  // PLACEHOLDER: The tab coordinator should be able to get the active webState
  // on its own.
  [self.activeTabCoordinator stop];
  [self removeChildCoordinator:self.activeTabCoordinator];
  TabCoordinator* tabCoordinator = [self newTabCoordinator];
  self.activeTabCoordinator = tabCoordinator;
  tabCoordinator.webState = self.webStateList.GetWebStateAt(index);
  tabCoordinator.presentationKey =
      [NSIndexPath indexPathForItem:index inSection:0];
  [self addChildCoordinator:tabCoordinator];
  [self deRegisterFromToolsMenuCommands];
  [tabCoordinator start];
}

- (void)closeTabGridTabAtIndex:(int)index {
  self.webStateList.DetachWebStateAt(index);
}

- (void)createAndShowNewTabInTabGrid {
  web::WebState::CreateParams webStateCreateParams(
      self.browser->browser_state());
  std::unique_ptr<web::WebState> webState =
      web::WebState::Create(webStateCreateParams);
  webState->SetWebUsageEnabled(true);
  self.webStateList.InsertWebState(
      self.webStateList.count(), std::move(webState),
      WebStateList::INSERT_FORCE_INDEX, WebStateOpener());
  [self showTabGridTabAtIndex:self.webStateList.count() - 1];
}

- (void)showTabGrid {
  [self.mediator takeSnapshotWithCache:self.snapshotCache];
  // This object should only ever have at most one child.
  DCHECK_LE(self.children.count, 1UL);
  BrowserCoordinator* child = [self.children anyObject];
  [child stop];
  [self removeChildCoordinator:child];
  [self registerForToolsMenuCommands];
}

#pragma mark - ToolsMenuCommands

- (void)showToolsMenu {
  ToolsCoordinator* toolsCoordinator = [[ToolsCoordinator alloc] init];
  [self addChildCoordinator:toolsCoordinator];
  ToolsMenuConfiguration* menuConfiguration =
      [[ToolsMenuConfiguration alloc] initWithDisplayView:nil];
  menuConfiguration.inTabSwitcher = YES;
  menuConfiguration.noOpenedTabs = self.browser->web_state_list().empty();
  menuConfiguration.inNewTabPage = NO;
  toolsCoordinator.toolsMenuConfiguration = menuConfiguration;
  [toolsCoordinator start];
  self.toolsMenuCoordinator = toolsCoordinator;
}

- (void)closeToolsMenu {
  [self.toolsMenuCoordinator stop];
  [self removeChildCoordinator:self.toolsMenuCoordinator];
}

#pragma mark - URLOpening

- (void)openURL:(NSURL*)URL {
  if (self.webStateList.active_index() == WebStateList::kInvalidIndex)
    return;
  [self.overlayCoordinator stop];
  [self removeOverlayCoordinator];
  web::WebState* activeWebState = self.webStateList.GetActiveWebState();
  web::NavigationManager::WebLoadParams params(net::GURLWithNSURL(URL));
  params.transition_type = ui::PAGE_TRANSITION_LINK;
  activeWebState->GetNavigationManager()->LoadURLWithParams(params);
  if (!self.children.count)
    [self showTabGridTabAtIndex:self.webStateList.active_index()];
}

#pragma mark - PrivateMethods

- (void)registerForContextMenuCommands {
  // Right now these are unregistered in |-stop|.  However, once incognito is
  // implemented, these commands will need to be unregistered before switching
  // to incognito mode, as "open in new tab" commands are meant to be handled
  // by the incognito TabGridCoordinator.
  [self.browser->dispatcher()
      startDispatchingToTarget:self
                   forSelector:@selector(openContextMenuLinkInNewTab:)];
  [self.browser->dispatcher()
      startDispatchingToTarget:self
                   forSelector:@selector(openContextMenuImageInNewTab:)];
}

- (void)registerForSettingsCommands {
  [self.browser->dispatcher() startDispatchingToTarget:self
                                           forSelector:@selector(showSettings)];
}

- (void)registerForTabGridCommands {
  [self.browser->dispatcher() startDispatchingToTarget:self
                                           forSelector:@selector(showTabGrid)];
  [self.browser->dispatcher()
      startDispatchingToTarget:self
                   forSelector:@selector(showTabGridTabAtIndex:)];
  [self.browser->dispatcher()
      startDispatchingToTarget:self
                   forSelector:@selector(closeTabGridTabAtIndex:)];
  [self.browser->dispatcher()
      startDispatchingToTarget:self
                   forSelector:@selector(createAndShowNewTabInTabGrid)];
}

- (void)registerForToolsMenuCommands {
  [self.browser->dispatcher()
      startDispatchingToTarget:self
                   forSelector:@selector(showToolsMenu)];
  [self.browser->dispatcher()
      startDispatchingToTarget:self
                   forSelector:@selector(closeToolsMenu)];
}

- (void)deRegisterFromToolsMenuCommands {
  [self.browser->dispatcher()
      stopDispatchingForSelector:@selector(showToolsMenu)];
  [self.browser->dispatcher()
      stopDispatchingForSelector:@selector(closeToolsMenu)];
}

// Creates and returns a tab coordinator based on whether the tap strip is
// enabled.
- (TabCoordinator*)newTabCoordinator {
  return base::FeatureList::IsEnabled(kTabFeaturesTabStrip)
             ? [[TabStripTabCoordinator alloc] init]
             : [[TabCoordinator alloc] init];
}

@end

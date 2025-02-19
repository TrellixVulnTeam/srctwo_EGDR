// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/content_suggestions/content_suggestions_coordinator.h"

#include "base/mac/foundation_util.h"
#include "base/mac/scoped_nsobject.h"
#include "base/metrics/histogram_macros.h"
#include "base/metrics/user_metrics.h"
#include "base/metrics/user_metrics_action.h"
#include "base/strings/sys_string_conversions.h"
#include "components/ntp_snippets/content_suggestions_service.h"
#include "components/ntp_snippets/remote/remote_suggestions_scheduler.h"
#include "components/ntp_tiles/metrics.h"
#include "components/ntp_tiles/most_visited_sites.h"
#include "components/reading_list/core/reading_list_model.h"
#include "components/strings/grit/components_strings.h"
#include "ios/chrome/browser/browser_state/chrome_browser_state.h"
#import "ios/chrome/browser/content_suggestions/content_suggestions_alert_factory.h"
#import "ios/chrome/browser/content_suggestions/content_suggestions_header_view_controller.h"
#import "ios/chrome/browser/content_suggestions/content_suggestions_mediator.h"
#import "ios/chrome/browser/content_suggestions/content_suggestions_metrics_recorder.h"
#import "ios/chrome/browser/content_suggestions/ntp_home_metrics.h"
#include "ios/chrome/browser/favicon/ios_chrome_large_icon_cache_factory.h"
#include "ios/chrome/browser/favicon/ios_chrome_large_icon_service_factory.h"
#include "ios/chrome/browser/favicon/large_icon_cache.h"
#import "ios/chrome/browser/metrics/new_tab_page_uma.h"
#include "ios/chrome/browser/ntp_snippets/ios_chrome_content_suggestions_service_factory.h"
#include "ios/chrome/browser/ntp_tiles/ios_most_visited_sites_factory.h"
#include "ios/chrome/browser/pref_names.h"
#include "ios/chrome/browser/reading_list/reading_list_model_factory.h"
#include "ios/chrome/browser/tabs/tab_constants.h"
#import "ios/chrome/browser/ui/alert_coordinator/alert_coordinator.h"
#import "ios/chrome/browser/ui/commands/browser_commands.h"
#import "ios/chrome/browser/ui/commands/open_new_tab_command.h"
#import "ios/chrome/browser/ui/commands/reading_list_add_command.h"
#import "ios/chrome/browser/ui/content_suggestions/cells/content_suggestions_gesture_commands.h"
#import "ios/chrome/browser/ui/content_suggestions/cells/content_suggestions_item.h"
#import "ios/chrome/browser/ui/content_suggestions/cells/content_suggestions_most_visited_item.h"
#import "ios/chrome/browser/ui/content_suggestions/content_suggestions_collection_utils.h"
#import "ios/chrome/browser/ui/content_suggestions/content_suggestions_commands.h"
#import "ios/chrome/browser/ui/content_suggestions/content_suggestions_data_sink.h"
#import "ios/chrome/browser/ui/content_suggestions/content_suggestions_header_synchronizer.h"
#import "ios/chrome/browser/ui/content_suggestions/content_suggestions_header_view_controller_delegate.h"
#import "ios/chrome/browser/ui/content_suggestions/content_suggestions_view_controller.h"
#import "ios/chrome/browser/ui/content_suggestions/content_suggestions_view_controller_audience.h"
#import "ios/chrome/browser/ui/content_suggestions/identifier/content_suggestion_identifier.h"
#import "ios/chrome/browser/ui/content_suggestions/ntp_home_constant.h"
#import "ios/chrome/browser/ui/ntp/google_landing_mediator.h"
#import "ios/chrome/browser/ui/ntp/new_tab_page_header_constants.h"
#import "ios/chrome/browser/ui/ntp/notification_promo_whats_new.h"
#import "ios/chrome/browser/ui/overscroll_actions/overscroll_actions_controller.h"
#import "ios/chrome/browser/ui/uikit_ui_util.h"
#import "ios/chrome/browser/ui/url_loader.h"
#import "ios/chrome/browser/web_state_list/web_state_list.h"
#include "ios/chrome/grit/ios_strings.h"
#import "ios/third_party/material_components_ios/src/components/Snackbar/src/MaterialSnackbar.h"
#import "ios/web/public/navigation_item.h"
#import "ios/web/public/navigation_manager.h"
#include "ios/web/public/referrer.h"
#import "ios/web/public/web_state/web_state.h"
#import "ios/web/public/web_state/web_state_observer_bridge.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/strings/grit/ui_strings.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace {

const char kNTPHelpURL[] = "https://support.google.com/chrome/?p=ios_new_tab";

// The What's New promo command that shows the Bookmarks Manager.
const char kBookmarkCommand[] = "bookmark";

// The What's New promo command that launches Rate This App.
const char kRateThisAppCommand[] = "ratethisapp";

}  // namespace

@interface ContentSuggestionsCoordinator ()<
    ContentSuggestionsCommands,
    ContentSuggestionsGestureCommands,
    ContentSuggestionsHeaderViewControllerCommandHandler,
    ContentSuggestionsHeaderViewControllerDelegate,
    ContentSuggestionsViewControllerAudience,
    CRWWebStateObserver,
    OverscrollActionsControllerDelegate>

@property(nonatomic, strong) AlertCoordinator* alertCoordinator;
@property(nonatomic, strong)
    ContentSuggestionsViewController* suggestionsViewController;
@property(nonatomic, strong)
    ContentSuggestionsMediator* contentSuggestionsMediator;
@property(nonatomic, strong) GoogleLandingMediator* googleLandingMediator;
@property(nonatomic, strong)
    ContentSuggestionsHeaderSynchronizer* headerCollectionInteractionHandler;
@property(nonatomic, strong) ContentSuggestionsMetricsRecorder* metricsRecorder;

// Redefined as readwrite.
@property(nonatomic, strong, readwrite)
    ContentSuggestionsHeaderViewController* headerController;

@end

@implementation ContentSuggestionsCoordinator {
  std::unique_ptr<web::WebStateObserverBridge> _webStateObserver;
}

@synthesize alertCoordinator = _alertCoordinator;
@synthesize browserState = _browserState;
@synthesize suggestionsViewController = _suggestionsViewController;
@synthesize URLLoader = _URLLoader;
@synthesize visible = _visible;
@synthesize contentSuggestionsMediator = _contentSuggestionsMediator;
@synthesize headerCollectionInteractionHandler =
    _headerCollectionInteractionHandler;
@synthesize headerController = _headerController;
@synthesize googleLandingMediator = _googleLandingMediator;
@synthesize webStateList = _webStateList;
@synthesize dispatcher = _dispatcher;
@synthesize delegate = _delegate;
@synthesize metricsRecorder = _metricsRecorder;

- (void)start {
  if (self.visible || !self.browserState) {
    // Prevent this coordinator from being started twice in a row or without a
    // browser state.
    return;
  }

  _visible = YES;

  web::WebState* webState = self.webStateList->GetActiveWebState();
  _webStateObserver =
      base::MakeUnique<web::WebStateObserverBridge>(webState, self);

  ntp_snippets::ContentSuggestionsService* contentSuggestionsService =
      IOSChromeContentSuggestionsServiceFactory::GetForBrowserState(
          self.browserState);
  contentSuggestionsService->remote_suggestions_scheduler()
      ->OnSuggestionsSurfaceOpened();
  PrefService* prefs =
      ios::ChromeBrowserState::FromBrowserState(self.browserState)->GetPrefs();
  bool contentSuggestionsEnabled =
      prefs->GetBoolean(prefs::kSearchSuggestEnabled);
  if (contentSuggestionsEnabled) {
    ntp_home::RecordNTPImpression(ntp_home::REMOTE_SUGGESTIONS);
  } else {
    ntp_home::RecordNTPImpression(ntp_home::LOCAL_SUGGESTIONS);
  }

  self.headerController = [[ContentSuggestionsHeaderViewController alloc] init];
  self.headerController.dispatcher = self.dispatcher;
  self.headerController.commandHandler = self;
  self.headerController.delegate = self;
  self.headerController.readingListModel =
      ReadingListModelFactory::GetForBrowserState(self.browserState);
  self.googleLandingMediator =
      [[GoogleLandingMediator alloc] initWithBrowserState:self.browserState
                                             webStateList:self.webStateList];
  self.googleLandingMediator.consumer = self.headerController;
  self.googleLandingMediator.dispatcher = self.dispatcher;
  [self.googleLandingMediator setUp];

  favicon::LargeIconService* largeIconService =
      IOSChromeLargeIconServiceFactory::GetForBrowserState(self.browserState);
  LargeIconCache* cache =
      IOSChromeLargeIconCacheFactory::GetForBrowserState(self.browserState);
  std::unique_ptr<ntp_tiles::MostVisitedSites> mostVisitedFactory =
      IOSMostVisitedSitesFactory::NewForBrowserState(self.browserState);
  self.contentSuggestionsMediator = [[ContentSuggestionsMediator alloc]
      initWithContentService:contentSuggestionsService
            largeIconService:largeIconService
              largeIconCache:cache
             mostVisitedSite:std::move(mostVisitedFactory)];
  self.contentSuggestionsMediator.commandHandler = self;
  self.contentSuggestionsMediator.headerProvider = self.headerController;
  self.metricsRecorder = [[ContentSuggestionsMetricsRecorder alloc] init];
  self.metricsRecorder.delegate = self.contentSuggestionsMediator;

  self.suggestionsViewController = [[ContentSuggestionsViewController alloc]
      initWithStyle:CollectionViewControllerStyleDefault];
  [self.suggestionsViewController
      setDataSource:self.contentSuggestionsMediator];
  self.suggestionsViewController.suggestionCommandHandler = self;
  self.suggestionsViewController.audience = self;
  self.suggestionsViewController.overscrollDelegate = self;
  self.suggestionsViewController.metricsRecorder = self.metricsRecorder;
  self.suggestionsViewController.containsToolbar = YES;

  [self.suggestionsViewController addChildViewController:self.headerController];
  [self.headerController
      didMoveToParentViewController:self.suggestionsViewController];

  self.headerCollectionInteractionHandler =
      [[ContentSuggestionsHeaderSynchronizer alloc]
          initWithCollectionController:self.suggestionsViewController
                      headerController:self.headerController];
}

- (void)stop {
  self.contentSuggestionsMediator = nil;
  self.alertCoordinator = nil;
  self.headerController = nil;
  [self.googleLandingMediator shutdown];
  self.googleLandingMediator = nil;
  _visible = NO;
  _webStateObserver.reset();
}

- (UIViewController*)viewController {
  return self.suggestionsViewController;
}

- (void)webState:(web::WebState*)webState didLoadPageWithSuccess:(BOOL)success {
  if (!success)
    return;

  web::NavigationManager* navigationManager = webState->GetNavigationManager();
  web::NavigationItem* item = navigationManager->GetVisibleItem();
  if (item && item->GetPageDisplayState().scroll_state().offset_y() > 0) {
    CGFloat offset = item->GetPageDisplayState().scroll_state().offset_y();
    UICollectionView* collection =
        self.suggestionsViewController.collectionView;
    // Don't set the offset such as the content of the collection is smaller
    // than the part of the collection which should be displayed with that
    // offset, taking into account the size of the toolbar.
    offset = MAX(0, MIN(offset, collection.contentSize.height -
                                    collection.bounds.size.height -
                                    ntp_header::kToolbarHeight));
    collection.contentOffset = CGPointMake(0, offset);
    // Update the constraints in case the omnibox needs to be moved.
    [self.suggestionsViewController updateConstraints];
  }
}

#pragma mark - ContentSuggestionsCommands

- (void)openReadingList {
  [self.dispatcher showReadingList];
}

- (void)openPageForItemAtIndexPath:(NSIndexPath*)indexPath {
  CollectionViewItem* item = [self.suggestionsViewController.collectionViewModel
      itemAtIndexPath:indexPath];
  ContentSuggestionsItem* suggestionItem =
      base::mac::ObjCCastStrict<ContentSuggestionsItem>(item);

  [self.metricsRecorder
         onSuggestionOpened:suggestionItem
                atIndexPath:indexPath
         sectionsShownAbove:[self.suggestionsViewController
                                numberOfSectionsAbove:indexPath.section]
      suggestionsShownAbove:[self.suggestionsViewController
                                numberOfSuggestionsAbove:indexPath.section]
                 withAction:WindowOpenDisposition::CURRENT_TAB];

  // Use a referrer with a specific URL to mark this entry as coming from
  // ContentSuggestions.
  web::Referrer referrer;
  referrer.url = GURL(tab_constants::kDoNotConsiderForMostVisited);

  [self.URLLoader loadURL:suggestionItem.URL
                 referrer:referrer
               transition:ui::PAGE_TRANSITION_AUTO_BOOKMARK
        rendererInitiated:NO];
  new_tab_page_uma::RecordAction(self.browserState,
                                 new_tab_page_uma::ACTION_OPENED_SUGGESTION);
}

- (void)openMostVisitedItem:(CollectionViewItem*)item
                    atIndex:(NSInteger)mostVisitedIndex {
  ContentSuggestionsMostVisitedItem* mostVisitedItem =
      base::mac::ObjCCastStrict<ContentSuggestionsMostVisitedItem>(item);

  [self logMostVisitedOpening:mostVisitedItem atIndex:mostVisitedIndex];

  [self.URLLoader loadURL:mostVisitedItem.URL
                 referrer:web::Referrer()
               transition:ui::PAGE_TRANSITION_AUTO_BOOKMARK
        rendererInitiated:NO];
}

- (void)displayContextMenuForSuggestion:(CollectionViewItem*)item
                                atPoint:(CGPoint)touchLocation
                            atIndexPath:(NSIndexPath*)indexPath
                        readLaterAction:(BOOL)readLaterAction {
  ContentSuggestionsItem* suggestionsItem =
      base::mac::ObjCCastStrict<ContentSuggestionsItem>(item);

  [self.metricsRecorder
      onMenuOpenedForSuggestion:suggestionsItem
                    atIndexPath:indexPath
          suggestionsShownAbove:[self.suggestionsViewController
                                    numberOfSuggestionsAbove:indexPath
                                                                 .section]];

  self.alertCoordinator = [ContentSuggestionsAlertFactory
      alertCoordinatorForSuggestionItem:suggestionsItem
                       onViewController:self.suggestionsViewController
                                atPoint:touchLocation
                            atIndexPath:indexPath
                        readLaterAction:readLaterAction
                         commandHandler:self];

  [self.alertCoordinator start];
}

- (void)displayContextMenuForMostVisitedItem:(CollectionViewItem*)item
                                     atPoint:(CGPoint)touchLocation
                                 atIndexPath:(NSIndexPath*)indexPath {
  ContentSuggestionsMostVisitedItem* mostVisitedItem =
      base::mac::ObjCCastStrict<ContentSuggestionsMostVisitedItem>(item);
  self.alertCoordinator = [ContentSuggestionsAlertFactory
      alertCoordinatorForMostVisitedItem:mostVisitedItem
                        onViewController:self.suggestionsViewController
                                 atPoint:touchLocation
                             atIndexPath:indexPath
                          commandHandler:self];

  [self.alertCoordinator start];
}

// TODO(crbug.com/761096) : Promo handling should be DRY and tested.
- (void)handlePromoTapped {
  NotificationPromoWhatsNew* notificationPromo =
      [self.contentSuggestionsMediator notificationPromo];
  DCHECK(notificationPromo);
  notificationPromo->HandleClosed();
  new_tab_page_uma::RecordAction(self.browserState,
                                 new_tab_page_uma::ACTION_OPENED_PROMO);

  if (notificationPromo->IsURLPromo()) {
    [self.URLLoader webPageOrderedOpen:notificationPromo->url()
                              referrer:web::Referrer()
                          inBackground:NO
                              appendTo:kCurrentTab];
    return;
  }

  if (notificationPromo->IsChromeCommandPromo()) {
    std::string command = notificationPromo->command();
    if (command == kBookmarkCommand) {
      [self.dispatcher showBookmarksManager];
    } else if (command == kRateThisAppCommand) {
      [self.dispatcher showRateThisAppDialog];
    } else {
      NOTREACHED() << "Promo command is not valid.";
    }
    return;
  }
  NOTREACHED() << "Promo type is neither URL or command.";
}

- (void)handleLearnMoreTapped {
  [self.URLLoader loadURL:GURL(kNTPHelpURL)
                 referrer:web::Referrer()
               transition:ui::PAGE_TRANSITION_LINK
        rendererInitiated:NO];
  new_tab_page_uma::RecordAction(self.browserState,
                                 new_tab_page_uma::ACTION_OPENED_LEARN_MORE);
}

#pragma mark - ContentSuggestionsGestureCommands

- (void)openNewTabWithSuggestionsItem:(ContentSuggestionsItem*)item
                            incognito:(BOOL)incognito {
  new_tab_page_uma::RecordAction(self.browserState,
                                 new_tab_page_uma::ACTION_OPENED_SUGGESTION);

  NSIndexPath* indexPath = [self.suggestionsViewController.collectionViewModel
      indexPathForItem:item];
  if (indexPath) {
    WindowOpenDisposition disposition =
        incognito ? WindowOpenDisposition::OFF_THE_RECORD
                  : WindowOpenDisposition::NEW_BACKGROUND_TAB;
    [self.metricsRecorder
           onSuggestionOpened:item
                  atIndexPath:indexPath
           sectionsShownAbove:[self.suggestionsViewController
                                  numberOfSectionsAbove:indexPath.section]
        suggestionsShownAbove:[self.suggestionsViewController
                                  numberOfSuggestionsAbove:indexPath.section]
                   withAction:disposition];
  }

  [self openNewTabWithURL:item.URL incognito:incognito];
}

- (void)addItemToReadingList:(ContentSuggestionsItem*)item {
  NSIndexPath* indexPath = [self.suggestionsViewController.collectionViewModel
      indexPathForItem:item];
  if (indexPath) {
    [self.metricsRecorder
           onSuggestionOpened:item
                  atIndexPath:indexPath
           sectionsShownAbove:[self.suggestionsViewController
                                  numberOfSectionsAbove:indexPath.section]
        suggestionsShownAbove:[self.suggestionsViewController
                                  numberOfSuggestionsAbove:indexPath.section]
                   withAction:WindowOpenDisposition::SAVE_TO_DISK];
  }

  self.contentSuggestionsMediator.readingListNeedsReload = YES;
  ReadingListAddCommand* command =
      [[ReadingListAddCommand alloc] initWithURL:item.URL title:item.title];
  [self.dispatcher addToReadingList:command];
}

- (void)dismissSuggestion:(ContentSuggestionsItem*)item
              atIndexPath:(NSIndexPath*)indexPath {
  NSIndexPath* itemIndexPath = indexPath;
  if (!itemIndexPath) {
    // If the caller uses a nil |indexPath|, find it from the model.
    itemIndexPath = [self.suggestionsViewController.collectionViewModel
        indexPathForItem:item];
  }

  [self.contentSuggestionsMediator dismissSuggestion:item.suggestionIdentifier];
  [self.suggestionsViewController dismissEntryAtIndexPath:itemIndexPath];
}

- (void)openNewTabWithMostVisitedItem:(ContentSuggestionsMostVisitedItem*)item
                            incognito:(BOOL)incognito
                              atIndex:(NSInteger)index {
  [self logMostVisitedOpening:item atIndex:index];
  [self openNewTabWithURL:item.URL incognito:incognito];
}

- (void)openNewTabWithMostVisitedItem:(ContentSuggestionsMostVisitedItem*)item
                            incognito:(BOOL)incognito {
  NSInteger index =
      [self.suggestionsViewController.collectionViewModel indexPathForItem:item]
          .item;
  [self openNewTabWithMostVisitedItem:item incognito:incognito atIndex:index];
}

- (void)removeMostVisited:(ContentSuggestionsMostVisitedItem*)item {
  base::RecordAction(base::UserMetricsAction("MostVisited_UrlBlacklisted"));
  [self.contentSuggestionsMediator blacklistMostVisitedURL:item.URL];
  [self showMostVisitedUndoForURL:item.URL];
}

#pragma mark - ContentSuggestionsHeaderViewControllerDelegate

- (BOOL)isContextMenuVisible {
  return self.alertCoordinator.isVisible;
}

- (BOOL)isScrolledToTop {
  return self.suggestionsViewController.scrolledToTop;
}

#pragma mark - ContentSuggestionsViewControllerAudience

- (void)contentOffsetDidChange {
  [self.delegate updateNtpBarShadowForPanelController:self];
}

- (void)promoShown {
  NotificationPromoWhatsNew* notificationPromo =
      [self.contentSuggestionsMediator notificationPromo];
  notificationPromo->HandleViewed();
  [self.headerController setPromoCanShow:notificationPromo->CanShow()];
}

#pragma mark - OverscrollActionsControllerDelegate

- (void)overscrollActionsController:(OverscrollActionsController*)controller
                   didTriggerAction:(OverscrollAction)action {
  switch (action) {
    case OverscrollAction::NEW_TAB: {
      [_dispatcher openNewTab:[OpenNewTabCommand command]];
    } break;
    case OverscrollAction::CLOSE_TAB: {
      [_dispatcher closeCurrentTab];
    } break;
    case OverscrollAction::REFRESH:
      [self reload];
      break;
    case OverscrollAction::NONE:
      NOTREACHED();
      break;
  }
}

- (BOOL)shouldAllowOverscrollActions {
  return YES;
}

- (UIView*)toolbarSnapshotView {
  return
      [[self.headerController toolBarView] snapshotViewAfterScreenUpdates:NO];
}

- (UIView*)headerView {
  return self.suggestionsViewController.view;
}

- (CGFloat)overscrollActionsControllerHeaderInset:
    (OverscrollActionsController*)controller {
  return 0;
}

- (CGFloat)overscrollHeaderHeight {
  return [self.headerController toolBarView].bounds.size.height;
}

#pragma mark - NewTabPagePanelProtocol

- (CGFloat)alphaForBottomShadow {
  UICollectionView* collection = self.suggestionsViewController.collectionView;

  NSInteger numberOfSection =
      [collection.dataSource numberOfSectionsInCollectionView:collection];

  NSInteger lastNonEmptySection = 0;
  NSInteger lastItemIndex = 0;
  for (NSInteger i = 0; i < numberOfSection; i++) {
    NSInteger itemsInSection = [collection.dataSource collectionView:collection
                                              numberOfItemsInSection:i];
    if (itemsInSection > 0) {
      // Some sections might be empty. Only consider the last non-empty one.
      lastNonEmptySection = i;
      lastItemIndex = itemsInSection - 1;
    }
  }
  if (lastNonEmptySection == 0)
    return 0;

  NSIndexPath* lastCellIndexPath =
      [NSIndexPath indexPathForItem:lastItemIndex
                          inSection:lastNonEmptySection];
  UICollectionViewLayoutAttributes* attributes =
      [collection layoutAttributesForItemAtIndexPath:lastCellIndexPath];
  CGRect lastCellFrame = attributes.frame;
  CGFloat pixelsBelowFrame =
      CGRectGetMaxY(lastCellFrame) - CGRectGetMaxY(collection.bounds);
  CGFloat alpha = pixelsBelowFrame / kNewTabPageDistanceToFadeShadow;
  return MIN(MAX(alpha, 0), 1);
}

- (UIView*)view {
  return self.suggestionsViewController.view;
}

- (void)reload {
  [self.contentSuggestionsMediator.dataSink reloadAllData];
}

- (void)wasShown {
  self.headerController.isShowing = YES;
  [self.suggestionsViewController.collectionView
          .collectionViewLayout invalidateLayout];
  [self.delegate updateNtpBarShadowForPanelController:self];
}

- (void)wasHidden {
  self.headerController.isShowing = NO;
}

- (void)dismissModals {
  [self.alertCoordinator stop];
  self.alertCoordinator = nil;
}

- (void)dismissKeyboard {
}

- (void)setScrollsToTop:(BOOL)enable {
}

#pragma mark - Private

// Opens the |URL| in a new tab |incognito| or not.
- (void)openNewTabWithURL:(const GURL&)URL incognito:(BOOL)incognito {
  // Open the tab in background if it is non-incognito only.
  [self.URLLoader webPageOrderedOpen:URL
                            referrer:web::Referrer()
                         inIncognito:incognito
                        inBackground:!incognito
                            appendTo:kCurrentTab];
}

// Logs a histogram due to a Most Visited item being opened.
- (void)logMostVisitedOpening:(ContentSuggestionsMostVisitedItem*)item
                      atIndex:(NSInteger)mostVisitedIndex {
  new_tab_page_uma::RecordAction(
      self.browserState, new_tab_page_uma::ACTION_OPENED_MOST_VISITED_ENTRY);
  base::RecordAction(base::UserMetricsAction("MobileNTPMostVisited"));

  ntp_tiles::metrics::RecordTileClick(mostVisitedIndex, item.source,
                                      [item tileType]);
}

// Shows a snackbar with an action to undo the removal of the most visited item
// with a |URL|.
- (void)showMostVisitedUndoForURL:(GURL)URL {
  GURL copiedURL = URL;

  MDCSnackbarMessageAction* action = [[MDCSnackbarMessageAction alloc] init];
  __weak ContentSuggestionsCoordinator* weakSelf = self;
  action.handler = ^{
    ContentSuggestionsCoordinator* strongSelf = weakSelf;
    if (!strongSelf)
      return;
    [strongSelf.contentSuggestionsMediator whitelistMostVisitedURL:copiedURL];
  };
  action.title = l10n_util::GetNSString(IDS_NEW_TAB_UNDO_THUMBNAIL_REMOVE);
  action.accessibilityIdentifier = @"Undo";

  TriggerHapticFeedbackForNotification(UINotificationFeedbackTypeSuccess);
  MDCSnackbarMessage* message = [MDCSnackbarMessage
      messageWithText:l10n_util::GetNSString(
                          IDS_IOS_NEW_TAB_MOST_VISITED_ITEM_REMOVED)];
  message.action = action;
  message.category = @"MostVisitedUndo";
  [MDCSnackbarManager showMessage:message];
}

@end

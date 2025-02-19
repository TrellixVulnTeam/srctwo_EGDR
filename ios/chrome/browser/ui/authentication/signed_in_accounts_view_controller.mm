// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/authentication/signed_in_accounts_view_controller.h"

#import "base/mac/foundation_util.h"
#include "components/signin/core/browser/account_tracker_service.h"
#include "components/signin/core/browser/profile_oauth2_token_service.h"
#include "components/signin/ios/browser/oauth2_token_service_observer_bridge.h"
#include "ios/chrome/browser/browser_state/chrome_browser_state.h"
#include "ios/chrome/browser/signin/account_tracker_service_factory.h"
#include "ios/chrome/browser/signin/authentication_service.h"
#include "ios/chrome/browser/signin/authentication_service_factory.h"
#include "ios/chrome/browser/signin/chrome_identity_service_observer_bridge.h"
#include "ios/chrome/browser/signin/oauth2_token_service_factory.h"
#import "ios/chrome/browser/ui/authentication/resized_avatar_cache.h"
#import "ios/chrome/browser/ui/collection_view/cells/collection_view_account_item.h"
#import "ios/chrome/browser/ui/collection_view/collection_view_controller.h"
#import "ios/chrome/browser/ui/collection_view/collection_view_model.h"
#import "ios/chrome/browser/ui/colors/MDCPalette+CrAdditions.h"
#import "ios/chrome/browser/ui/commands/application_commands.h"
#import "ios/chrome/browser/ui/util/constraints_ui_util.h"
#include "ios/chrome/grit/ios_chromium_strings.h"
#include "ios/chrome/grit/ios_strings.h"
#include "ios/public/provider/chrome/browser/chrome_browser_provider.h"
#import "ios/public/provider/chrome/browser/signin/chrome_identity.h"
#import "ios/public/provider/chrome/browser/signin/chrome_identity_service.h"
#include "ios/public/provider/chrome/browser/signin/signin_resources_provider.h"
#import "ios/third_party/material_components_ios/src/components/Buttons/src/MaterialButtons.h"
#import "ios/third_party/material_components_ios/src/components/Dialogs/src/MaterialDialogs.h"
#import "ios/third_party/material_components_ios/src/components/Palettes/src/MaterialPalettes.h"
#import "ios/third_party/material_components_ios/src/components/Typography/src/MaterialTypography.h"
#include "ui/base/l10n/l10n_util_mac.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace {

const int kMaxShownAccounts = 3;
const CGFloat kAccountsExtraBottomInset = 16;
const CGFloat kVerticalPadding = 24;
const CGFloat kButtonVerticalPadding = 16;
const CGFloat kHorizontalPadding = 24;
const CGFloat kAccountsHorizontalPadding = 8;
const CGFloat kButtonHorizontalPadding = 16;
const CGFloat kBetweenButtonsPadding = 8;
const CGFloat kMDCMinHorizontalPadding = 20;
const CGFloat kDialogMaxWidth = 328;

typedef NS_ENUM(NSInteger, SectionIdentifier) {
  SectionIdentifierAccounts = kSectionIdentifierEnumZero,
};

typedef NS_ENUM(NSInteger, ItemType) {
  ItemTypeAccount = kItemTypeEnumZero,
};

// Whether the Signed In Accounts view is currently being shown.
BOOL gSignedInAccountsViewControllerIsShown = NO;

}  // namespace

@interface SignedInAccountsCollectionViewController
    : CollectionViewController<ChromeIdentityServiceObserver> {
  ios::ChromeBrowserState* _browserState;  // Weak.
  std::unique_ptr<ChromeIdentityServiceObserverBridge> _identityServiceObserver;
  ResizedAvatarCache* _avatarCache;

  // Enable lookup of item corresponding to a given identity GAIA ID string.
  NSDictionary<NSString*, CollectionViewItem*>* _identityMap;
}
@end

@implementation SignedInAccountsCollectionViewController

- (instancetype)initWithBrowserState:(ios::ChromeBrowserState*)browserState {
  UICollectionViewLayout* layout = [[MDCCollectionViewFlowLayout alloc] init];
  self =
      [super initWithLayout:layout style:CollectionViewControllerStyleDefault];
  if (self) {
    _browserState = browserState;
    _avatarCache = [[ResizedAvatarCache alloc] init];
    _identityServiceObserver.reset(
        new ChromeIdentityServiceObserverBridge(self));
    [self loadModel];
  }
  return self;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  self.styler.shouldHideSeparators = YES;
  self.collectionView.backgroundColor = [UIColor clearColor];

  // Add an inset at the bottom so the user can see whether it is possible to
  // scroll to see additional accounts.
  UIEdgeInsets contentInset = self.collectionView.contentInset;
  contentInset.bottom += kAccountsExtraBottomInset;
  self.collectionView.contentInset = contentInset;
}

#pragma mark CollectionViewController

- (void)loadModel {
  [super loadModel];
  CollectionViewModel* model = self.collectionViewModel;
  NSMutableDictionary<NSString*, CollectionViewItem*>* mutableIdentityMap =
      [[NSMutableDictionary alloc] init];

  [model addSectionWithIdentifier:SectionIdentifierAccounts];
  ProfileOAuth2TokenService* oauth2_service =
      OAuth2TokenServiceFactory::GetForBrowserState(_browserState);
  AccountTrackerService* accountTracker =
      ios::AccountTrackerServiceFactory::GetForBrowserState(_browserState);
  for (const std::string& account_id : oauth2_service->GetAccounts()) {
    AccountInfo account = accountTracker->GetAccountInfo(account_id);
    ChromeIdentity* identity = ios::GetChromeBrowserProvider()
                                   ->GetChromeIdentityService()
                                   ->GetIdentityWithGaiaID(account.gaia);
    CollectionViewItem* item = [self accountItem:identity];
    [model addItem:item toSectionWithIdentifier:SectionIdentifierAccounts];
    [mutableIdentityMap setObject:item forKey:identity.gaiaID];
  }
  _identityMap = mutableIdentityMap;
}

#pragma mark Model objects

- (CollectionViewItem*)accountItem:(ChromeIdentity*)identity {
  CollectionViewAccountItem* item =
      [[CollectionViewAccountItem alloc] initWithType:ItemTypeAccount];
  [self updateAccountItem:item withIdentity:identity];
  return item;
}

- (void)updateAccountItem:(CollectionViewAccountItem*)item
             withIdentity:(ChromeIdentity*)identity {
  item.image = [_avatarCache resizedAvatarForIdentity:identity];
  item.text = [identity userFullName];
  item.detailText = [identity userEmail];
  item.chromeIdentity = identity;
}

#pragma mark MDCCollectionViewStylingDelegate

- (BOOL)collectionView:(UICollectionView*)collectionView
    shouldHideItemBackgroundAtIndexPath:(NSIndexPath*)indexPath {
  return YES;
}

- (CGFloat)collectionView:(UICollectionView*)collectionView
    cellHeightAtIndexPath:(NSIndexPath*)indexPath {
  return MDCCellDefaultOneLineWithAvatarHeight;
}

- (BOOL)collectionView:(UICollectionView*)collectionView
    hidesInkViewAtIndexPath:(NSIndexPath*)indexPath {
  return YES;
}

#pragma mark ChromeIdentityServiceObserver

- (void)profileUpdate:(ChromeIdentity*)identity {
  CollectionViewAccountItem* item =
      base::mac::ObjCCastStrict<CollectionViewAccountItem>(
          [_identityMap objectForKey:identity.gaiaID]);
  [self updateAccountItem:item withIdentity:identity];
  NSIndexPath* indexPath = [self.collectionViewModel indexPathForItem:item];
  [self.collectionView reloadItemsAtIndexPaths:@[ indexPath ]];
}

- (void)chromeIdentityServiceWillBeDestroyed {
  _identityServiceObserver.reset();
}

@end

@interface SignedInAccountsViewController ()<
    OAuth2TokenServiceObserverBridgeDelegate> {
  ios::ChromeBrowserState* _browserState;  // Weak.
  std::unique_ptr<OAuth2TokenServiceObserverBridge> _tokenServiceObserver;
  MDCDialogTransitionController* _transitionController;

  UILabel* _titleLabel;
  SignedInAccountsCollectionViewController* _accountsCollection;
  UILabel* _infoLabel;
  MDCButton* _primaryButton;
  MDCButton* _secondaryButton;
}
@property(nonatomic, readonly, weak) id<ApplicationSettingsCommands> dispatcher;
@end

@implementation SignedInAccountsViewController
@synthesize dispatcher = _dispatcher;

+ (BOOL)shouldBePresentedForBrowserState:
    (ios::ChromeBrowserState*)browserState {
  if (!browserState || browserState->IsOffTheRecord()) {
    return NO;
  }
  AuthenticationService* authService =
      AuthenticationServiceFactory::GetForBrowserState(browserState);
  return !gSignedInAccountsViewControllerIsShown &&
         authService->IsAuthenticated() && authService->HaveAccountsChanged();
}

#pragma mark Initialization

- (instancetype)initWithBrowserState:(ios::ChromeBrowserState*)browserState
                          dispatcher:
                              (id<ApplicationSettingsCommands>)dispatcher {
  self = [super initWithNibName:nil bundle:nil];
  if (self) {
    _browserState = browserState;
    _dispatcher = dispatcher;
    _tokenServiceObserver.reset(new OAuth2TokenServiceObserverBridge(
        OAuth2TokenServiceFactory::GetForBrowserState(_browserState), self));
    _transitionController = [[MDCDialogTransitionController alloc] init];
    self.modalPresentationStyle = UIModalPresentationCustom;
    self.transitioningDelegate = _transitionController;
  }
  return self;
}

- (void)dismiss {
  [self.presentingViewController dismissViewControllerAnimated:YES
                                                    completion:nil];
}

- (void)dealloc {
  [_primaryButton removeTarget:self
                        action:@selector(onPrimaryButtonPressed:)
              forControlEvents:UIControlEventTouchDown];
  [_secondaryButton removeTarget:self
                          action:@selector(onSecondaryButtonPressed:)
                forControlEvents:UIControlEventTouchDown];
}

#pragma mark UIViewController

- (CGSize)preferredContentSize {
  CGFloat width = MIN(kDialogMaxWidth,
                      self.presentingViewController.view.bounds.size.width -
                          2 * kMDCMinHorizontalPadding);
  OAuth2TokenService* token_service =
      OAuth2TokenServiceFactory::GetForBrowserState(_browserState);
  int shownAccounts =
      MIN(kMaxShownAccounts, token_service->GetAccounts().size());
  CGSize maxSize = CGSizeMake(width - 2 * kHorizontalPadding, CGFLOAT_MAX);
  CGSize buttonSize = [_primaryButton sizeThatFits:maxSize];
  CGSize infoSize = [_infoLabel sizeThatFits:maxSize];
  CGSize titleSize = [_titleLabel sizeThatFits:maxSize];
  CGFloat height = kVerticalPadding + titleSize.height + kVerticalPadding +
                   shownAccounts * MDCCellDefaultOneLineWithAvatarHeight +
                   kVerticalPadding + infoSize.height + kVerticalPadding +
                   buttonSize.height + kButtonVerticalPadding;
  return CGSizeMake(width, height);
}

- (void)viewDidLoad {
  [super viewDidLoad];

  self.view.backgroundColor = [UIColor whiteColor];

  _titleLabel = [[UILabel alloc] initWithFrame:CGRectZero];
  _titleLabel.text =
      l10n_util::GetNSString(IDS_IOS_SIGNED_IN_ACCOUNTS_VIEW_TITLE);
  _titleLabel.textColor = [[MDCPalette greyPalette] tint900];
  _titleLabel.font = [MDCTypography headlineFont];
  _titleLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:_titleLabel];

  _accountsCollection = [[SignedInAccountsCollectionViewController alloc]
      initWithBrowserState:_browserState];
  _accountsCollection.view.translatesAutoresizingMaskIntoConstraints = NO;
  [self addChildViewController:_accountsCollection];
  [self.view addSubview:_accountsCollection.view];
  [_accountsCollection didMoveToParentViewController:self];

  _infoLabel = [[UILabel alloc] initWithFrame:CGRectZero];
  _infoLabel.text =
      l10n_util::GetNSString(IDS_IOS_SIGNED_IN_ACCOUNTS_VIEW_INFO);
  _infoLabel.numberOfLines = 0;
  _infoLabel.textColor = [[MDCPalette greyPalette] tint700];
  _infoLabel.font = [MDCTypography body1Font];
  _infoLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:_infoLabel];

  _primaryButton = [[MDCFlatButton alloc] init];
  [_primaryButton addTarget:self
                     action:@selector(onPrimaryButtonPressed:)
           forControlEvents:UIControlEventTouchUpInside];
  [_primaryButton
      setTitle:l10n_util::GetNSString(IDS_IOS_SIGNED_IN_ACCOUNTS_VIEW_OK_BUTTON)
      forState:UIControlStateNormal];
  [_primaryButton setBackgroundColor:[[MDCPalette cr_bluePalette] tint500]
                            forState:UIControlStateNormal];
  [_primaryButton setTitleColor:[UIColor whiteColor]
                       forState:UIControlStateNormal];
  _primaryButton.underlyingColorHint = [UIColor blackColor];
  _primaryButton.inkColor = [UIColor colorWithWhite:1 alpha:0.2f];
  _primaryButton.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:_primaryButton];

  _secondaryButton = [[MDCFlatButton alloc] init];
  [_secondaryButton addTarget:self
                       action:@selector(onSecondaryButtonPressed:)
             forControlEvents:UIControlEventTouchUpInside];
  [_secondaryButton
      setTitle:l10n_util::GetNSString(
                   IDS_IOS_SIGNED_IN_ACCOUNTS_VIEW_SETTINGS_BUTTON)
      forState:UIControlStateNormal];
  [_secondaryButton setBackgroundColor:[UIColor whiteColor]
                              forState:UIControlStateNormal];
  [_secondaryButton setTitleColor:[[MDCPalette cr_bluePalette] tint500]
                         forState:UIControlStateNormal];
  _secondaryButton.underlyingColorHint = [UIColor whiteColor];
  _secondaryButton.inkColor = [UIColor colorWithWhite:0 alpha:0.06f];
  _secondaryButton.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:_secondaryButton];

  NSDictionary* views = @{
    @"title" : _titleLabel,
    @"accounts" : _accountsCollection.view,
    @"info" : _infoLabel,
    @"primaryButton" : _primaryButton,
    @"secondaryButton" : _secondaryButton,
  };
  NSDictionary* metrics = @{
    @"verticalPadding" : @(kVerticalPadding),
    @"accountsVerticalPadding" :
        @(kVerticalPadding - kAccountsExtraBottomInset),
    @"buttonVerticalPadding" : @(kButtonVerticalPadding),
    @"horizontalPadding" : @(kHorizontalPadding),
    @"accountsHorizontalPadding" : @(kAccountsHorizontalPadding),
    @"buttonHorizontalPadding" : @(kButtonHorizontalPadding),
    @"betweenButtonsPadding" : @(kBetweenButtonsPadding),
  };
  NSArray* constraints = @[
    @"V:|-(verticalPadding)-[title]-(verticalPadding)-[accounts]",
    @"V:[accounts]-(accountsVerticalPadding)-[info]",
    @"V:[info]-(verticalPadding)-[primaryButton]-(buttonVerticalPadding)-|",
    @"V:[info]-(verticalPadding)-[secondaryButton]-(buttonVerticalPadding)-|",
    @"H:|-(horizontalPadding)-[title]-(horizontalPadding)-|",
    @"H:|-(accountsHorizontalPadding)-[accounts]-(accountsHorizontalPadding)-|",
    @"H:|-(horizontalPadding)-[info]-(horizontalPadding)-|",
    @"H:[secondaryButton]-(betweenButtonsPadding)-[primaryButton]",
    @"H:[primaryButton]-(buttonHorizontalPadding)-|",
  ];

  ApplyVisualConstraintsWithMetrics(constraints, views, metrics);
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
  if ([self isBeingPresented] || [self isMovingToParentViewController]) {
    gSignedInAccountsViewControllerIsShown = YES;
  }
}

- (void)viewWillDisappear:(BOOL)animated {
  [super viewWillDisappear:animated];
  if ([self isBeingDismissed] || [self isMovingFromParentViewController]) {
    gSignedInAccountsViewControllerIsShown = NO;
  }
}

#pragma mark Events

- (void)onPrimaryButtonPressed:(id)sender {
  [self dismiss];
}

- (void)onSecondaryButtonPressed:(id)sender {
  [self dismiss];
  [self.dispatcher showAccountsSettings];
}

#pragma mark OAuth2TokenServiceObserverBridgeDelegate

- (void)onEndBatchChanges {
  ProfileOAuth2TokenService* tokenService =
      OAuth2TokenServiceFactory::GetForBrowserState(_browserState);
  if (tokenService->GetAccounts().empty()) {
    [self dismiss];
    return;
  }
  [_accountsCollection loadModel];
  [_accountsCollection.collectionView reloadData];
}

@end

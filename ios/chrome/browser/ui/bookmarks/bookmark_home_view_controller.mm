// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/bookmarks/bookmark_home_view_controller.h"

#include "base/metrics/user_metrics.h"
#include "base/strings/sys_string_conversions.h"
#include "components/bookmarks/browser/bookmark_model.h"
#include "components/strings/grit/components_strings.h"
#include "ios/chrome/browser/bookmarks/bookmark_model_factory.h"
#include "ios/chrome/browser/bookmarks/bookmark_new_generation_features.h"
#include "ios/chrome/browser/bookmarks/bookmarks_utils.h"
#include "ios/chrome/browser/browser_state/chrome_browser_state.h"
#import "ios/chrome/browser/metrics/new_tab_page_uma.h"
#import "ios/chrome/browser/ui/alert_coordinator/action_sheet_coordinator.h"
#import "ios/chrome/browser/ui/bookmarks/bars/bookmark_context_bar.h"
#import "ios/chrome/browser/ui/bookmarks/bars/bookmark_editing_bar.h"
#import "ios/chrome/browser/ui/bookmarks/bars/bookmark_navigation_bar.h"
#import "ios/chrome/browser/ui/bookmarks/bookmark_collection_view.h"
#import "ios/chrome/browser/ui/bookmarks/bookmark_controller_factory.h"
#import "ios/chrome/browser/ui/bookmarks/bookmark_edit_view_controller.h"
#import "ios/chrome/browser/ui/bookmarks/bookmark_folder_editor_view_controller.h"
#import "ios/chrome/browser/ui/bookmarks/bookmark_folder_view_controller.h"
#import "ios/chrome/browser/ui/bookmarks/bookmark_home_primary_view.h"
#import "ios/chrome/browser/ui/bookmarks/bookmark_home_view_controller_protected.h"
#import "ios/chrome/browser/ui/bookmarks/bookmark_home_waiting_view.h"
#import "ios/chrome/browser/ui/bookmarks/bookmark_menu_item.h"
#import "ios/chrome/browser/ui/bookmarks/bookmark_menu_view.h"
#include "ios/chrome/browser/ui/bookmarks/bookmark_model_bridge_observer.h"
#import "ios/chrome/browser/ui/bookmarks/bookmark_navigation_controller.h"
#import "ios/chrome/browser/ui/bookmarks/bookmark_panel_view.h"
#import "ios/chrome/browser/ui/bookmarks/bookmark_promo_controller.h"
#import "ios/chrome/browser/ui/bookmarks/bookmark_table_view.h"
#import "ios/chrome/browser/ui/bookmarks/bookmark_utils_ios.h"
#import "ios/chrome/browser/ui/rtl_geometry.h"
#import "ios/chrome/browser/ui/ui_util.h"
#import "ios/chrome/browser/ui/uikit_ui_util.h"
#import "ios/chrome/browser/ui/url_loader.h"
#import "ios/chrome/browser/ui/util/constraints_ui_util.h"
#include "ios/chrome/grit/ios_strings.h"
#import "ios/third_party/material_components_ios/src/components/Typography/src/MaterialTypography.h"
#include "ios/web/public/referrer.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/l10n/l10n_util_mac.h"

using bookmarks::BookmarkNode;

namespace {
// The width of the bookmark menu, displaying the different sections.
const CGFloat kMenuWidth = 264;

// The spacer between title and done button on the navigation bar.
const CGFloat kSpacer = 50;
}

@interface BookmarkHomeViewController ()<
    BookmarkCollectionViewDelegate,
    BookmarkEditViewControllerDelegate,
    BookmarkFolderEditorViewControllerDelegate,
    BookmarkFolderViewControllerDelegate,
    BookmarkModelBridgeObserver,
    BookmarkPromoControllerDelegate,
    BookmarkTableViewDelegate,
    ContextBarDelegate>

@end

@implementation BookmarkHomeViewController

@synthesize actionSheetCoordinator = _actionSheetCoordinator;
@synthesize bookmarkPromoController = _bookmarkPromoController;
@synthesize bookmarks = _bookmarks;
@synthesize browserState = _browserState;
@synthesize editing = _editing;
@synthesize editIndexPaths = _editIndexPaths;
@synthesize editingBar = _editingBar;
@synthesize editViewController = _editViewController;
@synthesize folderEditor = _folderEditor;
@synthesize folderSelector = _folderSelector;
@synthesize folderView = _folderView;
@synthesize loader = _loader;
@synthesize menuView = _menuView;
@synthesize navigationBar = _navigationBar;
@synthesize panelView = _panelView;
@synthesize primaryMenuItem = _primaryMenuItem;
@synthesize scrollToTop = _scrollToTop;
@synthesize sideSwipingPossible = _sideSwipingPossible;
@synthesize waitForModelView = _waitForModelView;
@synthesize homeDelegate = _homeDelegate;
@synthesize bookmarksTableView = _bookmarksTableView;
@synthesize contextBar = _contextBar;
@synthesize contextBarState = _contextBarState;

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

#pragma mark - Initializer

- (instancetype)initWithLoader:(id<UrlLoader>)loader
                  browserState:(ios::ChromeBrowserState*)browserState {
  DCHECK(browserState);
  self = [super initWithNibName:nil bundle:nil];
  if (self) {
    _editIndexPaths = [[NSMutableArray alloc] init];
    [self resetEditNodes];

    _browserState = browserState->GetOriginalChromeBrowserState();
    _loader = loader;

    _bookmarks = ios::BookmarkModelFactory::GetForBrowserState(browserState);

    _bridge.reset(new bookmarks::BookmarkModelBridge(self, _bookmarks));

    // It is important to initialize the promo controller with the browser state
    // passed in, as it could be incognito.
    _bookmarkPromoController =
        [[BookmarkPromoController alloc] initWithBrowserState:browserState
                                                     delegate:self];
  }
  return self;
}

- (void)setRootNode:(const bookmarks::BookmarkNode*)rootNode {
  _rootNode = rootNode;
}

#pragma mark - UIViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  if (!base::FeatureList::IsEnabled(kBookmarkNewGeneration)) {
    self.navigationBar =
        [[BookmarkNavigationBar alloc] initWithFrame:CGRectZero];
    [self.navigationBar setEditTarget:self
                               action:@selector(navigationBarWantsEditing:)];
    [self.navigationBar setBackTarget:self
                               action:@selector(navigationBarBack:)];
  } else {
    [self setupNavigationBar];
  }
}

- (void)viewWillLayoutSubviews {
  [super viewWillLayoutSubviews];
  if (base::FeatureList::IsEnabled(kBookmarkNewGeneration)) {
    // Resize the custom title view (placed as leftBarButtonItem) on the
    // navigation bar according to the space available, so that we truncate it
    // correctly.
    CGRect frame = self.navigationItem.leftBarButtonItem.customView.frame;
    NSDictionary* attributes = [self.navigationItem.rightBarButtonItem
        titleTextAttributesForState:UIControlStateNormal];
    CGFloat rightButtonWidth =
        attributes ? [self.navigationItem.rightBarButtonItem.title
                         sizeWithAttributes:attributes]
                         .width
                   : 0;
    frame.size.width = self.view.bounds.size.width - frame.origin.x -
                       rightButtonWidth - kSpacer;
    self.navigationItem.leftBarButtonItem.customView.frame = frame;
  }
}

#pragma mark - Public

- (void)dismissModals {
  [self.actionSheetCoordinator stop];
  self.actionSheetCoordinator = nil;
}

#pragma mark - Protected

- (void)loadBookmarkViews {
  if (base::FeatureList::IsEnabled(kBookmarkNewGeneration)) {
    [self loadBookmarkViewsForNewUI];
  } else {
    [self loadBookmarkViewsForOldUI];
  }
}

- (void)updatePrimaryMenuItem:(BookmarkMenuItem*)menuItem
                     animated:(BOOL)animated {
  DCHECK(menuItem.type == bookmarks::MenuItemFolder);
  if ([self.primaryMenuItem isEqual:menuItem])
    return;

  self.primaryMenuItem = menuItem;

  [self.folderView resetFolder:self.primaryMenuItem.folder];
  [self.folderView promoStateChangedAnimated:NO];

  [self.folderView changeOrientation:GetInterfaceOrientation()];
  [self.folderView setScrollsToTop:!self.scrollToTop];

  [self.menuView updatePrimaryMenuItem:self.primaryMenuItem];
}

- (void)loadWaitingView {
  DCHECK(!self.waitForModelView);
  DCHECK(self.view);

  // Present a waiting view.
  BookmarkHomeWaitingView* waitingView =
      [[BookmarkHomeWaitingView alloc] initWithFrame:self.view.bounds];
  self.waitForModelView = waitingView;
  [self.view addSubview:self.waitForModelView];
  [self.waitForModelView startWaiting];
}

- (CGFloat)menuWidth {
  return kMenuWidth;
}

- (void)cachePosition {
  if (self.folderView) {
    bookmark_utils_ios::CachePosition(
        [self.folderView contentPositionInPortraitOrientation],
        [self primaryMenuItem]);
  }
  // TODO(crbug.com/695749): Cache position for BookmarkTableView in new UI.
}

- (BOOL)shouldShowBackButtonOnNavigationBar {
  if (self.primaryMenuItem.type != bookmarks::MenuItemFolder)
    return NO;
  // The type is MenuItemFolder, so it is safe to access |folder|.
  const BookmarkNode* folder = self.primaryMenuItem.folder;
  // Show the back button iff the folder or its immediate parent is a permanent
  // primary folder.
  BOOL isTopFolder = IsPrimaryPermanentNode(folder, self.bookmarks) ||
                     IsPrimaryPermanentNode(folder->parent(), self.bookmarks);
  return !isTopFolder;
}

#pragma mark - Subclasses must override

- (void)showEditingBarAnimated:(BOOL)animated {
  NOTREACHED() << "Must be overridden by subclass";
}

- (void)hideEditingBarAnimated:(BOOL)animated {
  NOTREACHED() << "Must be overridden by subclass";
}

- (CGRect)editingBarFrame {
  NOTREACHED() << "Must be overridden by subclass";
  return CGRectZero;
}

- (ActionSheetCoordinator*)createActionSheetCoordinatorOnView:(UIView*)view {
  NOTREACHED() << "Must be overridden by subclass";
  return nil;
}

- (void)updateNavigationBarAnimated:(BOOL)animated
                        orientation:(UIInterfaceOrientation)orientation {
  NOTREACHED() << "Must be overridden by subclass";
}

#pragma mark - BookmarkPromoControllerDelegate

- (void)promoStateChanged:(BOOL)promoEnabled {
  if (base::FeatureList::IsEnabled(kBookmarkNewGeneration)) {
    [self.bookmarksTableView promoStateChangedAnimated:YES];
  } else {
    [self.folderView promoStateChangedAnimated:YES];
  }
}

#pragma mark Action sheet callbacks

// Enters into edit mode by selecting the given node corresponding to the
// given cell.
- (void)selectFirstNode:(const BookmarkNode*)node
               withCell:(UICollectionViewCell*)cell {
  DCHECK(!self.editing);
  [self insertEditNode:node atIndexPath:[self indexPathForCell:cell]];
  [self setEditing:YES animated:YES];
}

// Opens the folder move editor for the given node.
- (void)moveNodes:(const std::set<const BookmarkNode*>&)nodes {
  DCHECK(!self.folderSelector);
  DCHECK(nodes.size() > 0);
  const BookmarkNode* editedNode = *(nodes.begin());
  const BookmarkNode* selectedFolder = editedNode->parent();
  self.folderSelector = [[BookmarkFolderViewController alloc]
      initWithBookmarkModel:self.bookmarks
           allowsNewFolders:YES
                editedNodes:nodes
               allowsCancel:YES
             selectedFolder:selectedFolder];
  self.folderSelector.delegate = self;
  UINavigationController* navController = [[BookmarkNavigationController alloc]
      initWithRootViewController:self.folderSelector];
  [navController setModalPresentationStyle:UIModalPresentationFormSheet];
  [self presentViewController:navController animated:YES completion:NULL];
}

// Deletes the current node.
- (void)deleteNodes:(const std::set<const BookmarkNode*>&)nodes {
  DCHECK_GE(nodes.size(), 1u);
  bookmark_utils_ios::DeleteBookmarksWithUndoToast(nodes, self.bookmarks,
                                                   self.browserState);
}

// Opens the editor on the given node.
- (void)editNode:(const BookmarkNode*)node {
  DCHECK(!self.editViewController);
  DCHECK(!self.folderEditor);
  UIViewController* editorController = nil;
  if (node->is_folder()) {
    BookmarkFolderEditorViewController* folderEditor =
        [BookmarkFolderEditorViewController
            folderEditorWithBookmarkModel:self.bookmarks
                                   folder:node
                             browserState:self.browserState];
    folderEditor.delegate = self;
    self.folderEditor = folderEditor;
    editorController = folderEditor;
  } else {
    BookmarkEditViewController* controller =
        [[BookmarkEditViewController alloc] initWithBookmark:node
                                                browserState:self.browserState];
    self.editViewController = controller;
    self.editViewController.delegate = self;
    editorController = self.editViewController;
  }
  DCHECK(editorController);
  UINavigationController* navController = [[BookmarkNavigationController alloc]
      initWithRootViewController:editorController];
  navController.modalPresentationStyle = UIModalPresentationFormSheet;
  [self presentViewController:navController animated:YES completion:NULL];
}

#pragma mark - Navigation Bar Callbacks

- (void)navigationBarCancel:(id)sender {
  [self dismissWithURL:GURL()];
}

// Called when the edit button is pressed on the navigation bar.
- (void)navigationBarWantsEditing:(id)sender {
  DCHECK(self.primaryMenuItem.type == bookmarks::MenuItemFolder);
  const BookmarkNode* folder = self.primaryMenuItem.folder;
  BookmarkFolderEditorViewController* folderEditor =
      [BookmarkFolderEditorViewController
          folderEditorWithBookmarkModel:self.bookmarks
                                 folder:folder
                           browserState:self.browserState];
  folderEditor.delegate = self;
  self.folderEditor = folderEditor;

  BookmarkNavigationController* navController =
      [[BookmarkNavigationController alloc]
          initWithRootViewController:self.folderEditor];
  navController.modalPresentationStyle = UIModalPresentationFormSheet;
  [self presentViewController:navController animated:YES completion:NULL];
}

- (void)navigationBarBack:(id)sender {
  DCHECK([self shouldShowBackButtonOnNavigationBar]);

  // Go to the parent folder.
  DCHECK(self.primaryMenuItem.type == bookmarks::MenuItemFolder);
  const BookmarkNode* parentFolder = self.primaryMenuItem.folder->parent();
  const BookmarkNode* rootAncestor =
      RootLevelFolderForNode(parentFolder, self.bookmarks);
  BookmarkMenuItem* menuItem =
      [BookmarkMenuItem folderMenuItemForNode:parentFolder
                                 rootAncestor:rootAncestor];
  [self updatePrimaryMenuItem:menuItem animated:YES];
}

#pragma mark - BookmarkTableViewDelegate

- (void)bookmarkTableView:(BookmarkTableView*)view
    selectedUrlForNavigation:(const GURL&)url {
  [self dismissWithURL:url];
}

- (void)bookmarkTableView:(BookmarkTableView*)view
    selectedFolderForNavigation:(const bookmarks::BookmarkNode*)folder {
  BookmarkControllerFactory* bookmarkControllerFactory =
      [[BookmarkControllerFactory alloc] init];
  BookmarkHomeViewController* controller =
      (BookmarkHomeViewController*)[bookmarkControllerFactory
          bookmarkControllerWithBrowserState:self.browserState
                                      loader:_loader];
  [controller setRootNode:folder];
  controller.homeDelegate = self.homeDelegate;
  [self.navigationController pushViewController:controller animated:YES];
}

- (void)bookmarkTableView:(BookmarkTableView*)view
    selectedNodesForDeletion:
        (const std::set<const bookmarks::BookmarkNode*>&)nodes {
  [self deleteNodes:nodes];
}

- (BOOL)bookmarkTableViewShouldShowPromoCell:(BookmarkTableView*)tableView {
  return self.bookmarkPromoController.promoState;
}

- (void)bookmarkTableViewShowSignIn:(BookmarkTableView*)view {
  [self.bookmarkPromoController showSignIn];
}

- (void)bookmarkTableViewDismissPromo:(BookmarkTableView*)view {
  [self.bookmarkPromoController hidePromoCell];
}

- (void)bookmarkTableView:(BookmarkTableView*)view
        selectedEditNodes:
            (const std::set<const bookmarks::BookmarkNode*>&)nodes {
  // Early return if bookmarks table is not in edit mode.
  if (!self.bookmarksTableView.editing) {
    return;
  }

  if (nodes.size() == 0) {
    [self setContextBarState:BookmarksContextBarBeginSelection];
    return;
  }
  if (nodes.size() == 1) {
    const bookmarks::BookmarkNode* node = *nodes.begin();
    if (node->is_url()) {
      [self setContextBarState:BookmarksContextBarSingleURLSelection];
    } else if (node->is_folder()) {
      [self setContextBarState:BookmarksContextBarSingleFolderSelection];
    }
    return;
  }

  BOOL foundURL = NO;
  BOOL foundFolder = NO;
  for (std::set<const bookmarks::BookmarkNode*>::iterator i = nodes.begin();
       i != nodes.end(); ++i) {
    const bookmarks::BookmarkNode* node = *i;
    if (!foundURL && node->is_url()) {
      foundURL = YES;
    } else if (!foundFolder && node->is_folder()) {
      foundFolder = YES;
    }
    // Break early, if we found both types of nodes.
    if (foundURL && foundFolder) {
      break;
    }
  }

  // Only URLs are selected.
  if (foundURL && !foundFolder) {
    [self setContextBarState:BookmarksContextBarMultipleURLSelection];
    return;
  }
  // Only Folders are selected.
  if (!foundURL && foundFolder) {
    [self setContextBarState:BookmarksContextBarMultipleFolderSelection];
    return;
  }
  // Mixed selection.
  if (foundURL && foundFolder) {
    [self setContextBarState:BookmarksContextBarMixedSelection];
    return;
  }

  NOTREACHED();
  return;
}

- (void)bookmarkTableView:(BookmarkTableView*)view
    showContextMenuForNode:(const bookmarks::BookmarkNode*)node {
  if (node->is_url()) {
    [self presentViewController:[self contextMenuForSingleBookmarkURL:node]
                       animated:YES
                     completion:nil];
    return;
  }

  if (node->is_folder()) {
    [self presentViewController:[self contextMenuForSingleBookmarkFolder:node]
                       animated:YES
                     completion:nil];
    return;
  }
  NOTREACHED();
}

#pragma mark - BookmarkFolderViewControllerDelegate

- (void)folderPicker:(BookmarkFolderViewController*)folderPicker
    didFinishWithFolder:(const BookmarkNode*)folder {
  DCHECK(folder);
  DCHECK(!folder->is_url());
  DCHECK_GE(folderPicker.editedNodes.size(), 1u);

  bookmark_utils_ios::MoveBookmarksWithUndoToast(
      folderPicker.editedNodes, self.bookmarks, folder, self.browserState);

  [self setEditing:NO animated:NO];
  [self dismissViewControllerAnimated:YES completion:NULL];
  self.folderSelector.delegate = nil;
  self.folderSelector = nil;
}

- (void)folderPickerDidCancel:(BookmarkFolderViewController*)folderPicker {
  [self setEditing:NO animated:NO];
  [self dismissViewControllerAnimated:YES completion:NULL];
  self.folderSelector.delegate = nil;
  self.folderSelector = nil;
}

#pragma mark - BookmarkFolderEditorViewControllerDelegate

- (void)bookmarkFolderEditor:(BookmarkFolderEditorViewController*)folderEditor
      didFinishEditingFolder:(const BookmarkNode*)folder {
  DCHECK(folder);
  [self dismissViewControllerAnimated:YES completion:nil];
  self.folderEditor.delegate = nil;
  self.folderEditor = nil;
}

- (void)bookmarkFolderEditorDidDeleteEditedFolder:
    (BookmarkFolderEditorViewController*)folderEditor {
  [self dismissViewControllerAnimated:YES completion:nil];
  self.folderEditor.delegate = nil;
  self.folderEditor = nil;
}

- (void)bookmarkFolderEditorDidCancel:
    (BookmarkFolderEditorViewController*)folderEditor {
  [self dismissViewControllerAnimated:YES completion:nil];
  self.folderEditor.delegate = nil;
  self.folderEditor = nil;
}

#pragma mark - BookmarkEditViewControllerDelegate

- (BOOL)bookmarkEditor:(BookmarkEditViewController*)controller
    shoudDeleteAllOccurencesOfBookmark:(const BookmarkNode*)bookmark {
  return NO;
}

- (void)bookmarkEditorWantsDismissal:(BookmarkEditViewController*)controller {
  self.editViewController.delegate = nil;
  self.editViewController = nil;
  [self dismissViewControllerAnimated:YES completion:NULL];

  // The editViewController can be displayed from the menu button, or from the
  // edit button in edit mode. Either way, after it's dismissed, edit mode
  // should be off.
  [self setEditing:NO animated:NO];
}

#pragma mark - Edit

// Replaces |_editNodes| and |_editNodesOrdered| with new container objects.
- (void)resetEditNodes {
  _editNodes = std::set<const BookmarkNode*>();
  _editNodesOrdered = std::vector<const BookmarkNode*>();
  [self.editIndexPaths removeAllObjects];
}

// Adds |node| corresponding to a |cell| if it isn't already present.
- (void)insertEditNode:(const BookmarkNode*)node
           atIndexPath:(NSIndexPath*)indexPath {
  if (_editNodes.find(node) != _editNodes.end())
    return;
  _editNodes.insert(node);
  _editNodesOrdered.push_back(node);
  if (indexPath) {
    [self.editIndexPaths addObject:indexPath];
  } else {
    // Insert null to keep the index valid.
    [self.editIndexPaths addObject:[NSNull null]];
  }
}

// Removes |node| corresponding to a |cell| if it's present.
- (void)removeEditNode:(const BookmarkNode*)node
           atIndexPath:(NSIndexPath*)indexPath {
  if (_editNodes.find(node) == _editNodes.end())
    return;
  _editNodes.erase(node);
  std::vector<const BookmarkNode*>::iterator it =
      std::find(_editNodesOrdered.begin(), _editNodesOrdered.end(), node);
  DCHECK(it != _editNodesOrdered.end());
  _editNodesOrdered.erase(it);
  if (indexPath) {
    [self.editIndexPaths removeObject:indexPath];
  } else {
    // If we don't have the cell, we remove it by using its index.
    const NSUInteger index = std::distance(_editNodesOrdered.begin(), it);
    if (index < self.editIndexPaths.count) {
      [self.editIndexPaths removeObjectAtIndex:index];
    }
  }
}

// This method statelessly updates the editing top bar from |_editNodes| and
// |editing|.
- (void)updateEditingStateAnimated:(BOOL)animated {
  if (!self.editing) {
    [self hideEditingBarAnimated:animated];
    [self updateEditBarShadow];
    [self enableSideSwiping:YES];
    return;
  }

  if (!self.editingBar) {
    self.editingBar =
        [[BookmarkEditingBar alloc] initWithFrame:[self editingBarFrame]];
    [self.editingBar setCancelTarget:self action:@selector(editingBarCancel)];
    [self.editingBar setDeleteTarget:self action:@selector(editingBarDelete)];
    [self.editingBar setMoveTarget:self action:@selector(editingBarMove)];
    [self.editingBar setEditTarget:self action:@selector(editingBarEdit)];

    [self.view addSubview:self.editingBar];
    self.editingBar.alpha = 0;
    self.editingBar.hidden = YES;
  }

  int bookmarkCount = 0;
  int folderCount = 0;
  for (auto* node : _editNodes) {
    if (node->is_url())
      ++bookmarkCount;
    else
      ++folderCount;
  }
  [self.editingBar updateUIWithBookmarkCount:bookmarkCount
                                 folderCount:folderCount];

  [self showEditingBarAnimated:animated];
  [self updateEditBarShadow];
  [self enableSideSwiping:NO];
}

- (void)setEditing:(BOOL)editing animated:(BOOL)animated {
  if (_editing == editing)
    return;

  _editing = editing;

  if (editing) {
    self.bookmarkPromoController.promoState = NO;
  } else {
    // Only reset the editing state when leaving edit mode. This allows
    // subclasses to add nodes for editing before entering edit mode.
    [self resetEditNodes];
    [self.bookmarkPromoController updatePromoState];
  }

  [self updateEditingStateAnimated:animated];
  if ([[self primaryMenuItem] supportsEditing])
    [self.folderView setEditing:editing animated:animated];
}

- (void)updateEditBarShadow {
  [self.editingBar showShadow:self.editing];
}

#pragma mark Editing Bar Callbacks

// The cancel button was tapped on the editing bar.
- (void)editingBarCancel {
  [self setEditing:NO animated:YES];
}

// The move button was tapped on the editing bar.
- (void)editingBarMove {
  [self moveNodes:_editNodes];
}

// The delete button was tapped on the editing bar.
- (void)editingBarDelete {
  [self deleteSelectedNodes];
  [self setEditing:NO animated:YES];
}

// The edit button was tapped on the editing bar.
- (void)editingBarEdit {
  DCHECK_EQ(_editNodes.size(), 1u);
  const BookmarkNode* node = *(_editNodes.begin());
  [self editNode:node];
}

#pragma mark - BookmarkCollectionViewDelegate

// This class owns multiple views that have a delegate that conforms to
// BookmarkCollectionViewDelegate, or a subprotocol of
// BookmarkCollectionViewDelegate.
- (void)bookmarkCollectionView:(BookmarkCollectionView*)view
                          cell:(UICollectionViewCell*)cell
             addNodeForEditing:(const BookmarkNode*)node {
  [self insertEditNode:node atIndexPath:[self indexPathForCell:cell]];
  [self updateEditingStateAnimated:YES];
}

- (void)bookmarkCollectionView:(BookmarkCollectionView*)view
                          cell:(UICollectionViewCell*)cell
          removeNodeForEditing:(const BookmarkNode*)node {
  [self removeEditNode:node atIndexPath:[self indexPathForCell:cell]];
  if (_editNodes.size() == 0)
    [self setEditing:NO animated:YES];
  else
    [self updateEditingStateAnimated:YES];
}

- (const std::set<const BookmarkNode*>&)nodesBeingEdited {
  return _editNodes;
}

- (void)bookmarkCollectionViewDidScroll:(BookmarkCollectionView*)view {
  [self updateEditBarShadow];
}

- (void)bookmarkCollectionView:(BookmarkCollectionView*)view
              didLongPressCell:(UICollectionViewCell*)cell
                   forBookmark:(const BookmarkNode*)node {
  DCHECK(!self.editing);
  [self selectFirstNode:node withCell:cell];
}

- (BOOL)bookmarkCollectionViewShouldShowPromoCell:
    (BookmarkCollectionView*)collectionView {
  return self.bookmarkPromoController.promoState;
}

- (void)bookmarkCollectionViewShowSignIn:(BookmarkCollectionView*)view {
  [self.bookmarkPromoController showSignIn];
}

- (void)bookmarkCollectionViewDismissPromo:(BookmarkCollectionView*)view {
  [self.bookmarkPromoController hidePromoCell];
}

- (void)bookmarkCollectionView:(BookmarkCollectionView*)view
    selectedFolderForNavigation:(const BookmarkNode*)folder {
  BookmarkMenuItem* menuItem = nil;
  if (view == self.folderView) {
    const BookmarkNode* parent = RootLevelFolderForNode(folder, self.bookmarks);
    menuItem =
        [BookmarkMenuItem folderMenuItemForNode:folder rootAncestor:parent];
  } else {
    NOTREACHED();
    return;
  }
  [self updatePrimaryMenuItem:menuItem animated:YES];
}

- (void)bookmarkCollectionView:(BookmarkCollectionView*)view
      selectedUrlForNavigation:(const GURL&)url {
  [self dismissWithURL:url];
}

- (void)bookmarkCollectionView:(BookmarkCollectionView*)collectionView
          wantsMenuForBookmark:(const BookmarkNode*)node
                        onView:(UIView*)view
                       forCell:(BookmarkItemCell*)cell {
  DCHECK(!self.editViewController);
  DCHECK(!self.actionSheetCoordinator);
  self.actionSheetCoordinator = [self createActionSheetCoordinatorOnView:view];

  __weak BookmarkHomeViewController* weakSelf = self;

  // Select action.
  [self.actionSheetCoordinator
      addItemWithTitle:l10n_util::GetNSString(IDS_IOS_BOOKMARK_ACTION_SELECT)
                action:^{
                  [weakSelf selectFirstNode:node withCell:cell];
                  weakSelf.actionSheetCoordinator = nil;
                }
                 style:UIAlertActionStyleDefault];

  // Edit action.
  [self.actionSheetCoordinator
      addItemWithTitle:l10n_util::GetNSString(IDS_IOS_BOOKMARK_ACTION_EDIT)
                action:^{
                  [weakSelf editNode:node];
                  weakSelf.actionSheetCoordinator = nil;
                }
                 style:UIAlertActionStyleDefault];

  // Move action.
  [self.actionSheetCoordinator
      addItemWithTitle:l10n_util::GetNSString(IDS_IOS_BOOKMARK_ACTION_MOVE)
                action:^{
                  std::set<const BookmarkNode*> nodes;
                  nodes.insert(node);
                  [weakSelf moveNodes:nodes];
                  weakSelf.actionSheetCoordinator = nil;
                }
                 style:UIAlertActionStyleDefault];

  // Delete action.
  [self.actionSheetCoordinator
      addItemWithTitle:l10n_util::GetNSString(IDS_IOS_BOOKMARK_ACTION_DELETE)
                action:^{
                  std::set<const BookmarkNode*> nodes;
                  nodes.insert(node);
                  [weakSelf deleteNodes:nodes];
                  weakSelf.actionSheetCoordinator = nil;
                }
                 style:UIAlertActionStyleDestructive];

  // Cancel action.
  [self.actionSheetCoordinator
      addItemWithTitle:l10n_util::GetNSString(IDS_CANCEL)
                action:^{
                  weakSelf.actionSheetCoordinator = nil;
                }
                 style:UIAlertActionStyleCancel];

  [self.actionSheetCoordinator start];
}

#pragma mark - BookmarkModelBridgeObserver

- (void)bookmarkModelLoaded {
  if (![self isViewLoaded])
    return;

  DCHECK(self.waitForModelView);
  __weak BookmarkHomeViewController* weakSelf = self;
  [self.waitForModelView stopWaitingWithCompletion:^{
    BookmarkHomeViewController* strongSelf = weakSelf;
    // Early return if the controller has been deallocated.
    if (!strongSelf)
      return;
    [UIView animateWithDuration:0.2
        animations:^{
          strongSelf.waitForModelView.alpha = 0.0;
        }
        completion:^(BOOL finished) {
          [strongSelf.waitForModelView removeFromSuperview];
          strongSelf.waitForModelView = nil;
        }];
    [strongSelf loadBookmarkViews];
  }];
}

- (void)bookmarkNodeChanged:(const BookmarkNode*)node {
  // The title of the folder may have changed.
  if (self.primaryMenuItem.type == bookmarks::MenuItemFolder &&
      self.primaryMenuItem.folder == node) {
    UIInterfaceOrientation orient = GetInterfaceOrientation();
    [self updateNavigationBarAnimated:NO orientation:orient];
  }
}

- (void)bookmarkNodeChildrenChanged:(const BookmarkNode*)bookmarkNode {
  // The node has not changed, but the ordering and existence of its children
  // have changed.
}

- (void)bookmarkNode:(const BookmarkNode*)bookmarkNode
     movedFromParent:(const BookmarkNode*)oldParent
            toParent:(const BookmarkNode*)newParent {
  // The node has moved to a new parent folder.
}

- (void)bookmarkNodeDeleted:(const BookmarkNode*)node
                 fromFolder:(const BookmarkNode*)folder {
  [self removeEditNode:node atIndexPath:nil];
}

- (void)bookmarkModelRemovedAllNodes {
  // All non-permanent nodes have been removed.
  [self setEditing:NO animated:YES];
}

#pragma mark - Accessibility

- (BOOL)accessibilityPerformEscape {
  [self dismissWithURL:GURL()];
  return YES;
}

#pragma mark - private

// Set up UI views for the old UI.
- (void)loadBookmarkViewsForOldUI {
  LayoutRect menuLayout =
      LayoutRectMake(0, self.view.bounds.size.width, 0, self.menuWidth,
                     self.view.bounds.size.height);

  // Create menu view.
  self.menuView = [[BookmarkMenuView alloc]
      initWithBrowserState:self.browserState
                     frame:LayoutRectGetRect(menuLayout)];
  self.menuView.autoresizingMask =
      UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;

  // Create panel view.
  self.panelView = [[BookmarkPanelView alloc] initWithFrame:CGRectZero
                                              menuViewWidth:self.menuWidth];
  self.panelView.autoresizingMask =
      UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;

  // Create folder view.
  BookmarkCollectionView* view =
      [[BookmarkCollectionView alloc] initWithBrowserState:self.browserState
                                                     frame:CGRectZero];
  self.folderView = view;
  [self.folderView setEditing:self.editing animated:NO];
  self.folderView.autoresizingMask =
      UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;
  self.folderView.delegate = self;
}

// Set up UI views for the new UI.
- (void)loadBookmarkViewsForNewUI {
  // TODO(crbug.com/695749): Polish UI according to mocks.
  self.automaticallyAdjustsScrollViewInsets = NO;
  self.bookmarksTableView =
      [[BookmarkTableView alloc] initWithBrowserState:self.browserState
                                             delegate:self
                                             rootNode:_rootNode
                                                frame:self.view.bounds];
  [self.bookmarksTableView
      setAutoresizingMask:UIViewAutoresizingFlexibleWidth |
                          UIViewAutoresizingFlexibleHeight];
  [self.bookmarksTableView setTranslatesAutoresizingMaskIntoConstraints:NO];
  [self.view addSubview:self.bookmarksTableView];

  if (_rootNode != self.bookmarks->root_node()) {
    [self setupContextBar];
  }
}

// Set up context bar for the new UI.
- (void)setupContextBar {
  self.contextBar = [[BookmarkContextBar alloc] initWithFrame:CGRectZero];
  self.contextBar.delegate = self;
  [self.contextBar setTranslatesAutoresizingMaskIntoConstraints:NO];

  [self setContextBarState:BookmarksContextBarDefault];
  [self.view addSubview:self.contextBar];
}

// Set up navigation bar for the new UI.
- (void)setupNavigationBar {
  // Add custom back button.
  self.navigationItem.backBarButtonItem = [self customizedBackButton];

  // Add custom title.
  self.navigationItem.leftBarButtonItem = [self customizedNavigationTitle];

  // Add custom done button.
  self.navigationItem.rightBarButtonItem = [self customizedDoneButton];

  self.navigationController.navigationBar.tintColor = UIColor.blackColor;
  self.navigationController.navigationBar.barTintColor =
      bookmark_utils_ios::mainBackgroundColor();
  self.navigationController.navigationBar.translucent = NO;
}

- (UIBarButtonItem*)customizedBackButton {
  UIImage* backImage = [UIImage imageNamed:@"bookmark_gray_back"];
  // Set these two properties to customize the back button.
  [UINavigationBar appearance].backIndicatorImage = backImage;
  [UINavigationBar appearance].backIndicatorTransitionMaskImage = backImage;
  // Removes back button label.
  UIBarButtonItem* backButton =
      [[UIBarButtonItem alloc] initWithTitle:@""
                                       style:UIBarButtonItemStylePlain
                                      target:nil
                                      action:nil];
  backImage.accessibilityLabel =
      l10n_util::GetNSString(IDS_IOS_BOOKMARK_NEW_BACK_LABEL);
  return backButton;
}

- (UIBarButtonItem*)customizedNavigationTitle {
  NSDictionary* titleAttributes = @{
    NSForegroundColorAttributeName :
        [UIColor colorWithWhite:68 / 255.0 alpha:1.0],
    NSFontAttributeName : [MDCTypography titleFont]
  };
  UILabel* titleView = [[UILabel alloc] initWithFrame:CGRectZero];
  [titleView
      setAttributedText:
          [[NSAttributedString alloc]
              initWithString:bookmark_utils_ios::TitleForBookmarkNode(_rootNode)
                  attributes:titleAttributes]];
  [titleView sizeToFit];
  self.navigationItem.leftItemsSupplementBackButton = YES;
  return [[UIBarButtonItem alloc] initWithCustomView:titleView];
}

- (UIBarButtonItem*)customizedDoneButton {
  UIBarButtonItem* doneButton = [[UIBarButtonItem alloc]
      initWithTitle:l10n_util::GetNSString(IDS_IOS_NAVIGATION_BAR_DONE_BUTTON)
                        .localizedUppercaseString
              style:UIBarButtonItemStylePlain
             target:self
             action:@selector(navigationBarCancel:)];
  NSDictionary* attributes = @{
    NSForegroundColorAttributeName :
        [UIColor colorWithWhite:68 / 255.0 alpha:1.0],
    NSFontAttributeName : [MDCTypography buttonFont]
  };
  [doneButton setTitleTextAttributes:attributes forState:UIControlStateNormal];
  doneButton.accessibilityLabel =
      l10n_util::GetNSString(IDS_IOS_NAVIGATION_BAR_DONE_BUTTON);
  return doneButton;
}

// Saves the current position and asks the delegate to open the url, if delegate
// is set, otherwise opens the URL using loader.
- (void)dismissWithURL:(const GURL&)url {
  [self cachePosition];
  if (self.homeDelegate) {
    [self.homeDelegate bookmarkHomeViewControllerWantsDismissal:self
                                                navigationToUrl:url];
  } else {
    // Before passing the URL to the block, make sure the block has a copy of
    // the URL and not just a reference.
    const GURL localUrl(url);
    dispatch_async(dispatch_get_main_queue(), ^{
      [self loadURL:localUrl];
    });
  }
}

- (void)loadURL:(const GURL&)url {
  if (url == GURL() || url.SchemeIs(url::kJavaScriptScheme))
    return;

  new_tab_page_uma::RecordAction(self.browserState,
                                 new_tab_page_uma::ACTION_OPENED_BOOKMARK);
  base::RecordAction(
      base::UserMetricsAction("MobileBookmarkManagerEntryOpened"));
  [self.loader loadURL:url
               referrer:web::Referrer()
             transition:ui::PAGE_TRANSITION_AUTO_BOOKMARK
      rendererInitiated:NO];
}

- (void)enableSideSwiping:(BOOL)enable {
  if (self.sideSwipingPossible) {
    [self.panelView enableSideSwiping:enable];
  }
}

// Deletes the nodes, and presents a toast with an undo button.
- (void)deleteSelectedNodes {
  [self deleteNodes:_editNodes];
}

// Returns NSIndexPath for a given cell.
- (NSIndexPath*)indexPathForCell:(UICollectionViewCell*)cell {
  DCHECK(self.folderView.collectionView);
  NSIndexPath* indexPath =
      [self.folderView.collectionView indexPathForCell:cell];
  return indexPath;
}

- (void)updateViewConstraints {
  if (base::FeatureList::IsEnabled(kBookmarkNewGeneration)) {
    if (self.contextBar && self.bookmarksTableView) {
      NSDictionary* views = @{
        @"tableView" : self.bookmarksTableView,
        @"contextBar" : self.contextBar,
        @"topGuide" : self.topLayoutGuide,
      };
      NSArray* constraints = @[
        @"V:|[topGuide][tableView][contextBar(==48)]|",
        @"H:|[tableView]|",
        @"H:|[contextBar]|",
      ];
      ApplyVisualConstraints(constraints, views);
    } else if (self.bookmarksTableView) {
      NSDictionary* views = @{
        @"tableView" : self.bookmarksTableView,
        @"topGuide" : self.topLayoutGuide,
      };
      NSArray* constraints = @[
        @"V:|[topGuide][tableView]|",
        @"H:|[tableView]|",
      ];
      ApplyVisualConstraints(constraints, views);
    }
  }
  [super updateViewConstraints];
}

#pragma mark - ContextBarDelegate implementation

// Called when the leading button is clicked.
- (void)leadingButtonClicked {
  const std::set<const bookmarks::BookmarkNode*> nodes =
      [self.bookmarksTableView editNodes];
  switch (self.contextBarState) {
    case BookmarksContextBarDefault:
      // New Folder clicked.
      [self.bookmarksTableView addNewFolder];
      break;
    case BookmarksContextBarBeginSelection:
      // This must never happen, as the leading button is disabled at this
      // point.
      NOTREACHED();
      break;
    case BookmarksContextBarSingleURLSelection:
    case BookmarksContextBarMultipleURLSelection:
    case BookmarksContextBarSingleFolderSelection:
    case BookmarksContextBarMultipleFolderSelection:
    case BookmarksContextBarMixedSelection:
      // Delete clicked.
      [self deleteNodes:nodes];
      break;
    case BookmarksContextBarNone:
    default:
      NOTREACHED();
  }
}

// Called when the center button is clicked.
- (void)centerButtonClicked {
  const std::set<const bookmarks::BookmarkNode*> nodes =
      [self.bookmarksTableView editNodes];
  // Center button is shown and is clickable only when at least
  // one node is selected.
  DCHECK(nodes.size() > 0);
  switch (self.contextBarState) {
    case BookmarksContextBarDefault:
      // Center button is disabled in default state.
      NOTREACHED();
      break;
    case BookmarksContextBarBeginSelection:
      // Center button is disabled in start state.
      NOTREACHED();
      break;
    case BookmarksContextBarSingleURLSelection:
      // More clicked, show action sheet with context menu.
      [self presentViewController:
                [self contextMenuForSingleBookmarkURL:*(nodes.begin())]
                         animated:YES
                       completion:nil];
      break;
    case BookmarksContextBarMultipleURLSelection:
      // More clicked, show action sheet with context menu.
      [self presentViewController:[self contextMenuForMultipleBookmarkURLs]
                         animated:YES
                       completion:nil];
      break;
    case BookmarksContextBarSingleFolderSelection:
      // Edit clicked, open the editor.
      [self editNode:*(nodes.begin())];
      break;
    case BookmarksContextBarMultipleFolderSelection:
    case BookmarksContextBarMixedSelection:
      // More clicked, show action sheet with context menu.
      [self presentViewController:
                [self contextMenuForMixedAndMultiFolderSelection:nodes]
                         animated:YES
                       completion:nil];
      break;
    case BookmarksContextBarNone:
    default:
      NOTREACHED();
  }
}

// Called when the trailing button, "Select" or "Cancel" is clicked.
- (void)trailingButtonClicked {
  // Toggle edit mode.
  [self.bookmarksTableView setEditing:!self.bookmarksTableView.editing];
  [self setContextBarState:self.bookmarksTableView.editing
                               ? BookmarksContextBarBeginSelection
                               : BookmarksContextBarDefault];
}

#pragma mark - ContextBarStates

// Customizes the context bar buttons based the |state| passed in.
- (void)setContextBarState:(BookmarksContextBarState)state {
  _contextBarState = state;
  switch (state) {
    case BookmarksContextBarDefault:
      [self setBookmarksContextBarButtonsDefaultState];
      break;
    case BookmarksContextBarBeginSelection:
      [self setBookmarksContextBarSelectionStartState];
      break;
    case BookmarksContextBarSingleURLSelection:
    case BookmarksContextBarMultipleURLSelection:
    case BookmarksContextBarMultipleFolderSelection:
    case BookmarksContextBarMixedSelection:
      // Reset to start state, and then override with customizations that apply.
      [self setBookmarksContextBarSelectionStartState];
      [self.contextBar setButtonEnabled:YES forButton:ContextBarCenterButton];
      [self.contextBar setButtonEnabled:YES forButton:ContextBarLeadingButton];
      break;
    case BookmarksContextBarSingleFolderSelection:
      // Reset to start state, and then override with customizations that apply.
      [self setBookmarksContextBarSelectionStartState];
      [self.contextBar setButtonTitle:l10n_util::GetNSString(
                                          IDS_IOS_BOOKMARK_CONTEXT_BAR_EDIT)
                            forButton:ContextBarCenterButton];
      [self.contextBar setButtonEnabled:YES forButton:ContextBarCenterButton];
      [self.contextBar setButtonEnabled:YES forButton:ContextBarLeadingButton];
      break;
    case BookmarksContextBarNone:
    default:
      break;
  }
}

- (void)setBookmarksContextBarButtonsDefaultState {
  // Set New Folder button
  [self.contextBar setButtonTitle:l10n_util::GetNSString(
                                      IDS_IOS_BOOKMARK_CONTEXT_BAR_NEW_FOLDER)
                        forButton:ContextBarLeadingButton];
  [self.contextBar setButtonVisibility:YES forButton:ContextBarLeadingButton];
  [self.contextBar setButtonEnabled:YES forButton:ContextBarLeadingButton];
  [self.contextBar setButtonStyle:ContextBarButtonStyleDefault
                        forButton:ContextBarLeadingButton];

  // Set Center button to invisible.
  [self.contextBar setButtonVisibility:NO forButton:ContextBarCenterButton];

  // Set Select button.
  [self.contextBar
      setButtonTitle:l10n_util::GetNSString(IDS_IOS_BOOKMARK_CONTEXT_BAR_SELECT)
           forButton:ContextBarTrailingButton];
  [self.contextBar setButtonVisibility:YES forButton:ContextBarTrailingButton];
  [self.contextBar setButtonEnabled:YES forButton:ContextBarTrailingButton];
}

- (void)setBookmarksContextBarSelectionStartState {
  // Disabled Delete button.
  [self.contextBar
      setButtonTitle:l10n_util::GetNSString(IDS_IOS_BOOKMARK_CONTEXT_BAR_DELETE)
           forButton:ContextBarLeadingButton];
  [self.contextBar setButtonVisibility:YES forButton:ContextBarLeadingButton];
  [self.contextBar setButtonEnabled:NO forButton:ContextBarLeadingButton];
  [self.contextBar setButtonStyle:ContextBarButtonStyleDelete
                        forButton:ContextBarLeadingButton];

  // Disabled More button.
  [self.contextBar
      setButtonTitle:l10n_util::GetNSString(IDS_IOS_BOOKMARK_CONTEXT_BAR_MORE)
           forButton:ContextBarCenterButton];
  [self.contextBar setButtonVisibility:YES forButton:ContextBarCenterButton];
  [self.contextBar setButtonEnabled:NO forButton:ContextBarCenterButton];

  // Enabled Cancel button.
  [self.contextBar setButtonTitle:l10n_util::GetNSString(IDS_CANCEL)
                        forButton:ContextBarTrailingButton];
  [self.contextBar setButtonVisibility:YES forButton:ContextBarTrailingButton];
  [self.contextBar setButtonEnabled:YES forButton:ContextBarTrailingButton];
}

#pragma mark - Context Menu

- (UIAlertController*)contextMenuForMultipleBookmarkURLs {
  UIAlertController* alert = [UIAlertController
      alertControllerWithTitle:nil
                       message:nil
                preferredStyle:UIAlertControllerStyleActionSheet];
  alert.view.accessibilityIdentifier = @"bookmark_context_menu";

  UIAlertAction* cancelAction =
      [UIAlertAction actionWithTitle:l10n_util::GetNSString(IDS_CANCEL)
                               style:UIAlertActionStyleCancel
                             handler:nil];

  UIAlertAction* openAllAction = [UIAlertAction
      actionWithTitle:l10n_util::GetNSString(IDS_IOS_BOOKMARK_CONTEXT_MENU_OPEN)
                style:UIAlertActionStyleDefault
              handler:nil];

  UIAlertAction* openInIncognitoAction = [UIAlertAction
      actionWithTitle:l10n_util::GetNSString(
                          IDS_IOS_BOOKMARK_CONTEXT_MENU_OPEN_INCOGNITO)
                style:UIAlertActionStyleDefault
              handler:nil];

  UIAlertAction* moveAction = [UIAlertAction
      actionWithTitle:l10n_util::GetNSString(IDS_IOS_BOOKMARK_CONTEXT_MENU_MOVE)
                style:UIAlertActionStyleDefault
              handler:nil];
  [alert addAction:openAllAction];
  [alert addAction:openInIncognitoAction];
  [alert addAction:moveAction];
  [alert addAction:cancelAction];
  return alert;
}

- (UIAlertController*)contextMenuForSingleBookmarkURL:
    (const BookmarkNode*)node {
  __weak BookmarkHomeViewController* weakSelf = self;
  UIAlertController* alert = [UIAlertController
      alertControllerWithTitle:nil
                       message:nil
                preferredStyle:UIAlertControllerStyleActionSheet];
  alert.view.accessibilityIdentifier = @"bookmark_context_menu";

  UIAlertAction* cancelAction =
      [UIAlertAction actionWithTitle:l10n_util::GetNSString(IDS_CANCEL)
                               style:UIAlertActionStyleCancel
                             handler:nil];

  UIAlertAction* editAction = [UIAlertAction
      actionWithTitle:l10n_util::GetNSString(IDS_IOS_BOOKMARK_CONTEXT_MENU_EDIT)
                style:UIAlertActionStyleDefault
              handler:^(UIAlertAction* _Nonnull action) {
                [weakSelf editNode:node];
              }];

  void (^copyHandler)(UIAlertAction*) = ^(UIAlertAction*) {
    UIPasteboard* pasteboard = [UIPasteboard generalPasteboard];
    std::string urlString = node->url().possibly_invalid_spec();
    pasteboard.string = base::SysUTF8ToNSString(urlString);
  };
  UIAlertAction* copyAction = [UIAlertAction
      actionWithTitle:l10n_util::GetNSString(IDS_IOS_BOOKMARK_CONTEXT_MENU_COPY)
                style:UIAlertActionStyleDefault
              handler:copyHandler];

  UIAlertAction* openInIncognitoAction = [UIAlertAction
      actionWithTitle:l10n_util::GetNSString(
                          IDS_IOS_BOOKMARK_CONTEXT_MENU_OPEN_INCOGNITO)
                style:UIAlertActionStyleDefault
              handler:nil];
  [alert addAction:editAction];
  [alert addAction:copyAction];
  [alert addAction:openInIncognitoAction];
  [alert addAction:cancelAction];
  return alert;
}

- (UIAlertController*)contextMenuForSingleBookmarkFolder:
    (const BookmarkNode*)node {
  __weak BookmarkHomeViewController* weakSelf = self;
  UIAlertController* alert = [UIAlertController
      alertControllerWithTitle:nil
                       message:nil
                preferredStyle:UIAlertControllerStyleActionSheet];
  alert.view.accessibilityIdentifier = @"bookmark_context_menu";

  UIAlertAction* cancelAction =
      [UIAlertAction actionWithTitle:l10n_util::GetNSString(IDS_CANCEL)
                               style:UIAlertActionStyleCancel
                             handler:nil];

  UIAlertAction* editAction = [UIAlertAction
      actionWithTitle:l10n_util::GetNSString(IDS_IOS_BOOKMARK_CONTEXT_MENU_EDIT)
                style:UIAlertActionStyleDefault
              handler:^(UIAlertAction* _Nonnull action) {
                [weakSelf editNode:node];
              }];

  void (^moveHandler)(UIAlertAction*) = ^(UIAlertAction*) {
    std::set<const BookmarkNode*> nodes;
    nodes.insert(node);
    [weakSelf moveNodes:nodes];
  };
  UIAlertAction* moveAction = [UIAlertAction
      actionWithTitle:l10n_util::GetNSString(IDS_IOS_BOOKMARK_CONTEXT_MENU_MOVE)
                style:UIAlertActionStyleDefault
              handler:moveHandler];
  [alert addAction:editAction];
  [alert addAction:moveAction];
  [alert addAction:cancelAction];
  return alert;
}

- (UIAlertController*)contextMenuForMixedAndMultiFolderSelection:
    (const std::set<const bookmarks::BookmarkNode*>&)nodes {
  UIAlertController* alert = [UIAlertController
      alertControllerWithTitle:nil
                       message:nil
                preferredStyle:UIAlertControllerStyleActionSheet];
  alert.view.accessibilityIdentifier = @"bookmark_context_menu";

  UIAlertAction* cancelAction =
      [UIAlertAction actionWithTitle:l10n_util::GetNSString(IDS_CANCEL)
                               style:UIAlertActionStyleCancel
                             handler:nil];

  UIAlertAction* moveAction = [UIAlertAction
      actionWithTitle:l10n_util::GetNSString(IDS_IOS_BOOKMARK_CONTEXT_MENU_MOVE)
                style:UIAlertActionStyleDefault
              handler:nil];
  [alert addAction:moveAction];
  [alert addAction:cancelAction];
  return alert;
}

@end

@implementation BookmarkHomeViewController (ExposedForTesting)

- (const std::set<const BookmarkNode*>&)editNodes {
  return _editNodes;
}

@end

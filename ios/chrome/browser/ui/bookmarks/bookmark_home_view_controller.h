// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_BOOKMARKS_HOME_VIEW_CONTROLLER_H_
#define IOS_CHROME_BROWSER_UI_BOOKMARKS_HOME_VIEW_CONTROLLER_H_

#import <UIKit/UIKit.h>

#include <set>
#include <vector>

@protocol UrlLoader;
class GURL;

namespace ios {
class ChromeBrowserState;
}  // namespace ios

namespace bookmarks {
class BookmarkModelBridge;
class BookmarkNode;
}  // namespace bookmarks

@class BookmarkHomeViewController;

@protocol BookmarkHomeViewControllerDelegate
// The view controller wants to be dismissed.
// If |url| != GURL(), then the user has selected |url| for navigation.
- (void)bookmarkHomeViewControllerWantsDismissal:
            (BookmarkHomeViewController*)controller
                                 navigationToUrl:(const GURL&)url;
@end

// Class to navigate the bookmark hierarchy, needs subclassing for tablet /
// handset case.
@interface BookmarkHomeViewController : UIViewController {
 @protected
  // The following 2 ivars both represent the set of nodes being edited.
  // The set is for fast lookup.
  // The vector maintains the order that edit nodes were added.
  // Use the relevant instance methods to modify these two ivars in tandem.
  // DO NOT modify these two ivars directly.
  std::set<const bookmarks::BookmarkNode*> _editNodes;
  std::vector<const bookmarks::BookmarkNode*> _editNodesOrdered;

  // Bridge to register for bookmark changes.
  std::unique_ptr<bookmarks::BookmarkModelBridge> _bridge;

  // The root node, whose child nodes are shown in the bookmark table view.
  const bookmarks::BookmarkNode* _rootNode;
}

- (instancetype)initWithNibName:(NSString*)nibNameOrNil
                         bundle:(NSBundle*)nibBundleOrNil NS_UNAVAILABLE;
- (instancetype)initWithCoder:(NSCoder*)aDecoder NS_UNAVAILABLE;
- (instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithLoader:(id<UrlLoader>)loader
                  browserState:(ios::ChromeBrowserState*)browserState
    NS_DESIGNATED_INITIALIZER;

// Setter to set _rootNode value.
- (void)setRootNode:(const bookmarks::BookmarkNode*)rootNode;

// Delegate for presenters. Note that this delegate is currently being set only
// in case of handset, and not tablet. In the future it will be used by both
// cases.
@property(nonatomic, weak) id<BookmarkHomeViewControllerDelegate> homeDelegate;

// Dismisses any modal interaction elements. Note that this
// method is currently used in case of handset only. In the future it
// will be used by both cases.
- (void)dismissModals;
- (void)setRootNode:(const bookmarks::BookmarkNode*)rootNode;

@end

@interface BookmarkHomeViewController (ExposedForTesting)

- (const std::set<const bookmarks::BookmarkNode*>&)editNodes;

@end

#endif  // IOS_CHROME_BROWSER_UI_BOOKMARKS_HOME_VIEW_CONTROLLER_H_

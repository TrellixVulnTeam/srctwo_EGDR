// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_FIND_IN_PAGE_FIND_IN_PAGE_CONTROLLER_H_
#define IOS_CHROME_BROWSER_FIND_IN_PAGE_FIND_IN_PAGE_CONTROLLER_H_

#import <Foundation/Foundation.h>

#include "base/ios/block_types.h"

namespace web {
class WebState;
}

@class FindInPageModel;

extern NSString* const kFindBarTextFieldWillBecomeFirstResponderNotification;
extern NSString* const kFindBarTextFieldDidResignFirstResponderNotification;

@protocol FindInPageControllerDelegate<NSObject>
// Informs the delegate when the scroll position is about to be changed on the
// page.
- (void)willAdjustScrollPosition;
@end

@interface FindInPageController : NSObject

// Find In Page model.
@property(nonatomic, readonly, strong) FindInPageModel* findInPageModel;

// Designated initializer.
- (id)initWithWebState:(web::WebState*)webState
              delegate:(id<FindInPageControllerDelegate>)delegate;
// Inject the find in page scripts into the web state.
- (void)initFindInPage;
// Is Find In Page available right now (given the state of the WebState)?
- (BOOL)canFindInPage;
// Find |query| in page, update model with results of find. Calls
// |completionHandler| after the find operation is complete. |completionHandler|
// can be nil.
- (void)findStringInPage:(NSString*)query
       completionHandler:(ProceduralBlock)completionHandler;
// Move to the next find result based on |-findInPageModel|, and scroll to
// match. Calls |completionHandler| when the next string has been found.
// |completionHandler| can be nil.
- (void)findNextStringInPageWithCompletionHandler:
    (ProceduralBlock)completionHandler;
// Move to the previous find result based on |-findInPageModel|. Calls
// |completionHandler| when the previous string has been found.
// |completionHandler| can be nil.
- (void)findPreviousStringInPageWithCompletionHandler:
    (ProceduralBlock)completionHandler;
// Disable find in page script and model. Calls |completionHandler| once the
// model has been disabled and cleanup is complete. |completionHandler| can be
// nil.
- (void)disableFindInPageWithCompletionHandler:
    (ProceduralBlock)completionHandler;

// Save search term to Paste UIPasteboard.
- (void)saveSearchTerm;
// Restore search term from Paste UIPasteboard, updating findInPageModel.
- (void)restoreSearchTerm;

// Instructs the controller to detach itself from the web state.
- (void)detachFromWebState;

// Sets the search term to |string|. Stored until the application quit.
+ (void)setSearchTerm:(NSString*)string;
// The search term, stored until the application quit.
+ (NSString*)searchTerm;
@end

#endif  // IOS_CHROME_BROWSER_FIND_IN_PAGE_FIND_IN_PAGE_CONTROLLER_H_

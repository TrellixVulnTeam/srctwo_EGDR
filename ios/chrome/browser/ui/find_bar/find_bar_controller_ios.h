// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_FIND_BAR_FIND_BAR_CONTROLLER_IOS_H_
#define IOS_CHROME_BROWSER_UI_FIND_BAR_FIND_BAR_CONTROLLER_IOS_H_

#import <UIKit/UIKit.h>

@protocol BrowserCommands;
@class FindInPageModel;

// The a11y ID of the find-in-page bar.
extern NSString* const kFindInPageContainerViewId;

@interface FindBarControllerIOS : NSObject

// The main view, for both iPhone or iPad.
@property(nonatomic, readonly, strong) IBOutlet UIView* view;

// The dispatcher for sending browser commands.
@property(nonatomic, weak) id<BrowserCommands> dispatcher;

// Init with incognito style.
- (instancetype)initWithIncognito:(BOOL)isIncognito;
// Current input search term.
- (NSString*)searchTerm;
// Update view based on model. If |focusTextfield| is YES, focus the
// textfield. Updates the results count and, if |initialUpdate| is true, fills
// the text field with search term from the model.
- (void)updateView:(FindInPageModel*)model
     initialUpdate:(BOOL)initialUpdate
    focusTextfield:(BOOL)focusTextfield;

// Updates the results count in Find Bar.
- (void)updateResultsCount:(FindInPageModel*)model;

// Display find bar view. On iPad, it makes best effort to horizontally align
// find bar with alignment frame, unless alignment frame is too wide or too
// narrow. When too wide, the find bar is shorter and right-aligned. When
// |alignmentFrame| is too narrow, the find bar will horizontally fill |view|.
// If |selectText| flag is YES, the text in the text input field will be
// selected.
- (void)addFindBarView:(BOOL)animate
              intoView:(UIView*)view
             withFrame:(CGRect)frame
        alignWithFrame:(CGRect)alignmentFrame
            selectText:(BOOL)selectText;
// Hide find bar view.
- (void)hideFindBarView:(BOOL)animate;
// Hide the keyboard when the find next/previous buttons are pressed.
- (IBAction)hideKeyboard:(id)sender;
// Indicates that Find in Page is shown. When true, |view| is guaranteed not to
// be nil.
- (BOOL)isFindInPageShown;
// Indicates that the Find in Page text field is first responder.
- (BOOL)isFocused;

@end

#endif  // IOS_CHROME_BROWSER_UI_FIND_BAR_FIND_BAR_CONTROLLER_IOS_H_

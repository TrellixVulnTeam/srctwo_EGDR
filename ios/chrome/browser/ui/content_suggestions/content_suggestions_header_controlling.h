// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_CONTENT_SUGGESTIONS_CONTENT_SUGGESTIONS_HEADER_CONTROLLING_H_
#define IOS_CHROME_BROWSER_UI_CONTENT_SUGGESTIONS_CONTENT_SUGGESTIONS_HEADER_CONTROLLING_H_

#import <UIKit/UIKit.h>

@protocol ContentSuggestionsCollectionSynchronizing;

// Controller for the ContentSuggestions header.
@protocol ContentSuggestionsHeaderControlling

// Synchronizer for the header controller, allowing it to synchronize with its
// collection.
@property(nonatomic, weak) id<ContentSuggestionsCollectionSynchronizing>
    collectionSynchronizer;

// Updates the iPhone fakebox's frame based on the current scroll view |offset|
// and |width|. |width| can be 0 to use the current view width.
- (void)updateFakeOmniboxForOffset:(CGFloat)offset width:(CGFloat)width;

// Updates the fakeomnibox's width in order to be adapted to the new |width|,
// without taking the y-position into account.
- (void)updateFakeOmniboxForWidth:(CGFloat)width;

// Unfocuses the omnibox.
- (void)unfocusOmnibox;

// Calls layoutIfNeeded on the header.
- (void)layoutHeader;

// Returns the Y value to use for the scroll view's contentOffset when scrolling
// the omnibox to the top of the screen.
- (CGFloat)pinnedOffsetY;

// Whether the omnibox is currently focused.
- (BOOL)isOmniboxFocused;

// Returns the height of the header.
- (CGFloat)headerHeight;

@end

#endif  // IOS_CHROME_BROWSER_UI_CONTENT_SUGGESTIONS_CONTENT_SUGGESTIONS_HEADER_CONTROLLING_H_

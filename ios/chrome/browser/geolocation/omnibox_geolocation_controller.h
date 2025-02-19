// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_GEOLOCATION_OMNIBOX_GEOLOCATION_CONTROLLER_H_
#define IOS_CHROME_BROWSER_GEOLOCATION_OMNIBOX_GEOLOCATION_CONTROLLER_H_

#import <Foundation/Foundation.h>

#include "ui/base/page_transition_types.h"

class GURL;
@class Tab;

namespace ios {
class ChromeBrowserState;
}

namespace web {
class NavigationItem;
}

// Manages using the current device location for omnibox search queries.
@interface OmniboxGeolocationController : NSObject

// Returns singleton object for this class.
+ (OmniboxGeolocationController*)sharedInstance;

// Triggers the iOS system prompt to authorize the use of location, if the
// authorization is not yet determined.
// |newUser| specifies whether this is a new user or an existing user
- (void)triggerSystemPromptForNewUser:(BOOL)newUser;

// Notifies the receiver that the location bar became the first responder.
- (void)locationBarDidBecomeFirstResponder:
    (ios::ChromeBrowserState*)browserState;

// Notifies the receiver that the location bar stopped being the first
// responder.
- (void)locationBarDidResignFirstResponder:
    (ios::ChromeBrowserState*)browserState;

// Notifies the receiver that the user submitted a URL via the location bar.
- (void)locationBarDidSubmitURL:(const GURL&)url
                     transition:(ui::PageTransition)transition
                   browserState:(ios::ChromeBrowserState*)browserState;

// Adds the current device location to |item| if |item| represents an Omnibox
// query that's eligible for location. Returns |YES| if the current device
// location was added to |item|; returns |NO| otherwise.
- (BOOL)addLocationToNavigationItem:(web::NavigationItem*)item
                       browserState:(ios::ChromeBrowserState*)browserState;

// Notifies the receiver that the browser finished loading the page for |tab|.
// |loadSuccess| whether the tab loaded successfully
- (void)finishPageLoadForTab:(Tab*)tab loadSuccess:(BOOL)loadSuccess;

@end

#endif  // IOS_CHROME_BROWSER_GEOLOCATION_OMNIBOX_GEOLOCATION_CONTROLLER_H_

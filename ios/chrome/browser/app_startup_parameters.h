// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_APP_STARTUP_PARAMETERS_H_
#define IOS_CHROME_BROWSER_APP_STARTUP_PARAMETERS_H_

#import <Foundation/Foundation.h>

#include <map>

enum NTPTabOpeningPostOpeningAction {
  // No action should be done
  NO_ACTION = 0,
  START_VOICE_SEARCH,
  START_QR_CODE_SCANNER,
  FOCUS_OMNIBOX,
  NTP_TAB_OPENING_POST_OPENING_ACTION_COUNT,
};

class GURL;

// This class stores all the parameters relevant to the app startup in case
// of launch from another app.
@interface AppStartupParameters : NSObject

// The URL that should be opened. This may not always be the same URL as the one
// that was receieved. The reason for this is in the case of Universal Link
// navigation where we may want to open up a fallback URL e.g., the New Tab
// Page instead of the actual universal link.
@property(nonatomic, readonly, assign) const GURL& externalURL;

// The URL query string parameters in the case that the app was launched as a
// result of Universal Link navigation. The map associates query string
// parameters with their corresponding value.
@property(nonatomic, assign) std::map<std::string, std::string>
    externalURLParams;

//// Boolean to track if a voice search is requested at startup.
//@property(nonatomic, readwrite, assign) BOOL launchVoiceSearch;
// Boolean to track if the app should launch in incognito mode.
@property(nonatomic, readwrite, assign) BOOL launchInIncognito;
@property(nonatomic, readwrite, assign)
    NTPTabOpeningPostOpeningAction postOpeningAction;
// Boolean to track if a Payment Request response is requested at startup.
@property(nonatomic, readwrite, assign) BOOL completePaymentRequest;

- (instancetype)init NS_UNAVAILABLE;

- (instancetype)initWithExternalURL:(const GURL&)externalURL
    NS_DESIGNATED_INITIALIZER;

- (instancetype)initWithUniversalLink:(const GURL&)universalLink;

@end

#endif  // IOS_CHROME_BROWSER_APP_STARTUP_PARAMETERS_H_

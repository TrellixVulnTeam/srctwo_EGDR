// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_SHARED_CHROME_BROWSER_UI_OMNIBOX_LOCATION_BAR_DELEGATE_H_
#define IOS_SHARED_CHROME_BROWSER_UI_OMNIBOX_LOCATION_BAR_DELEGATE_H_

#import <Foundation/Foundation.h>

#include "ui/base/page_transition_types.h"

class GURL;
class ToolbarModel;

namespace web {
class WebState;
}

// Delegate for LocationBarController objects.  Used to provide the location bar
// a way to open URLs and otherwise interact with the browser.
@protocol LocationBarDelegate
- (void)loadGURLFromLocationBar:(const GURL&)url
                     transition:(ui::PageTransition)transition;
- (void)locationBarHasBecomeFirstResponder;
- (void)locationBarHasResignedFirstResponder;
- (void)locationBarBeganEdit;
- (web::WebState*)getWebState;
- (ToolbarModel*)toolbarModel;
@end

#endif  // IOS_SHARED_CHROME_BROWSER_UI_OMNIBOX_LOCATION_BAR_DELEGATE_H_

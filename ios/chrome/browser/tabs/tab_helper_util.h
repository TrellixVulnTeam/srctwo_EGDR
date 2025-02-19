// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_TABS_TAB_HELPER_UTIL_H_
#define IOS_CHROME_BROWSER_TABS_TAB_HELPER_UTIL_H_

namespace web {
class WebState;
}

// Attaches tab helpers to WebState.
void AttachTabHelpers(web::WebState* web_state);

#endif  // IOS_CHROME_BROWSER_TABS_TAB_HELPER_UTIL_H_

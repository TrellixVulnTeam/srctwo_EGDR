// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_WEB_SHELL_TEST_APP_NAVIGATION_TEST_UTIL_H_
#define IOS_WEB_SHELL_TEST_APP_NAVIGATION_TEST_UTIL_H_

#include "url/gurl.h"

namespace web {
namespace shell_test_util {

// Loads |url| in the current WebState with transition of type
// ui::PAGE_TRANSITION_TYPED.
void LoadUrl(const GURL& url);

// Returns true if the current page in the current WebState is loading.
bool IsLoading();

}  // namespace shell_test_util
}  // namespace web

#endif  // IOS_WEB_SHELL_TEST_APP_NAVIGATION_TEST_UTIL_H_

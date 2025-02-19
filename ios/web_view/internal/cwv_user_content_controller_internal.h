// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_WEB_VIEW_INTERNAL_CWV_USER_CONTENT_CONTROLLER_INTERNAL_H_
#define IOS_WEB_VIEW_INTERNAL_CWV_USER_CONTENT_CONTROLLER_INTERNAL_H_

#import "ios/web_view/public/cwv_user_content_controller.h"

@class CWVWebViewConfiguration;

@interface CWVUserContentController ()

- (nonnull instancetype)initWithConfiguration:
    (nonnull __weak CWVWebViewConfiguration*)configuration;

@end

#endif  // IOS_WEB_VIEW_INTERNAL_CWV_USER_CONTENT_CONTROLLER_INTERNAL_H_

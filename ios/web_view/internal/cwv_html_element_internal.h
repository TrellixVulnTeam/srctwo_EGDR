// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_WEB_VIEW_INTERNAL_CWV_HTML_ELEMENT_INTERNAL_H_
#define IOS_WEB_VIEW_INTERNAL_CWV_HTML_ELEMENT_INTERNAL_H_

#import "ios/web_view/public/cwv_html_element.h"

@interface CWVHTMLElement ()

- (nonnull instancetype)init NS_UNAVAILABLE;

- (nonnull instancetype)initWithHyperlink:(nullable NSURL*)hyperlink
                              mediaSource:(nullable NSURL*)mediaSource
                                     text:(nullable NSString*)text
    NS_DESIGNATED_INITIALIZER;

@end

#endif  // IOS_WEB_VIEW_INTERNAL_CWV_HTML_ELEMENT_INTERNAL_H_

// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_WEB_VIEW_PUBLIC_CWV_HTML_ELEMENT_H_
#define IOS_WEB_VIEW_PUBLIC_CWV_HTML_ELEMENT_H_

#import <Foundation/Foundation.h>

#import "cwv_export.h"

// Encapsulates information about HTML element. Used in
// delegate methods.
CWV_EXPORT
@interface CWVHTMLElement : NSObject

// |href| property of an HTML element.
@property(nullable, copy, readonly) NSURL* hyperlink;
// |src| property of an HTML element.
@property(nullable, copy, readonly) NSURL* mediaSource;
// |innerText| property of an HTML element.
@property(nullable, copy, readonly) NSString* text;
@end

#endif  // IOS_WEB_VIEW_PUBLIC_CWV_HTML_ELEMENT_H_

// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_WEB_PUBLIC_WEB_KIT_CONSTANTS_H_
#define IOS_WEB_PUBLIC_WEB_KIT_CONSTANTS_H_

// This header defines missing symbols from WebKit.
// See WebKitErrors.h on Mac SDK.

namespace web {

// Indicates WebKit errors.
extern const char kWebKitErrorDomain[];

// MIME type is not supported.
const long kWebKitErrorCannotShowMimeType = 100;

// Can not change location URL.
const long kWebKitErrorCannotShowUrl = 101;

// Frame load was interrupted by a policy change (f.e. by rejecting the load in
// decidePolicyForNavigationAction: or decidePolicyForNavigationResponse:
// WKNavigationDelegate callback).
const long kWebKitErrorFrameLoadInterruptedByPolicyChange = 102;

// Undocumented iOS-specific WebKit error.
const long kWebKitErrorUnsafePort = 103;
const long kWebKitErrorPlugInLoadFailed = 204;

}  // namespace web

#endif  // IOS_WEB_PUBLIC_WEB_KIT_CONSTANTS_H_

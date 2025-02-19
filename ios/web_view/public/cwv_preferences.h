// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_WEB_VIEW_PUBLIC_CWV_PREFERENCES_H_
#define IOS_WEB_VIEW_PUBLIC_CWV_PREFERENCES_H_

#import <Foundation/Foundation.h>

#import "cwv_export.h"

// Preferences for user settings.
CWV_EXPORT
@interface CWVPreferences : NSObject

// Whether or not translation as a feature is turned on.
// Because translate settings are shared from incognito to non-incognito, this
// has no effect if this instance is from an incognito CWVWebViewConfiguration.
@property(nonatomic, assign, getter=isTranslationEnabled)
    BOOL translationEnabled;

- (instancetype)init NS_UNAVAILABLE;

// Resets all translation settings back to default. In particular, this will
// change all translation policies back to CWVTranslationPolicyAsk.
// Because translate settings are shared from incognito to non-incognito, this
// has no effect if this instance is from an incognito CWVWebViewConfiguration.
- (void)resetTranslationSettings;

@end

#endif  // IOS_WEB_VIEW_PUBLIC_CWV_PREFERENCES_H_

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/public/provider/chrome/browser/ui/test_styled_text_field.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

@implementation TestStyledTextField
@synthesize placeholderStyle = _placeholderStyle;
@synthesize textValidator = _textValidator;

- (void)setUseErrorStyling:(BOOL)error {
}
@end

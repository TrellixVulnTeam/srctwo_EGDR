// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_SHARE_EXTENSION_UI_UTIL_H_
#define IOS_CHROME_SHARE_EXTENSION_UI_UTIL_H_

namespace ui_util {

// Standard animation timing for the extension.
extern const CGFloat kAnimationDuration;

// Returns the closest pixel-aligned value less than |value|, taking the scale
// factor into account. At a scale of 1, equivalent to floor().
CGFloat AlignValueToPixel(CGFloat value);

// Creates constraints so that |filler| fills entirely |container| and make them
// active.
void ConstrainAllSidesOfViewToView(UIView* container, UIView* filler);

}  // namespace ui_util

#endif  // IOS_CHROME_SHARE_EXTENSION_UI_UTIL_H_

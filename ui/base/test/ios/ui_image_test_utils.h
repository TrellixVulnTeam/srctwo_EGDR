// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_TEST_IOS_UI_IMAGE_TEST_UTILS_H_
#define UI_BASE_TEST_IOS_UI_IMAGE_TEST_UTILS_H_

#import <UIKit/UIKit.h>

namespace ui {
namespace test {
namespace uiimage_utils {

// Returns a new UIImage of size |size| with a solid color of |color|.
UIImage* UIImageWithSizeAndSolidColor(CGSize const& size, UIColor* color);

// Disclaimer, this is a testing function with plenty of limitations:
// Requires the UIImages to be backed by a CGImage, ignores colorspace, may
// return false negatives, not efficient, and probably other things.
//
// Returns whether the pixels in |image_1| are equal to the pixels in
// |image_2|.
// This is unlike UIImage's |isEqual:| method which only returns YES if the
// memory backing the images is the same (see Apple's response to
// radar://30188145).
bool UIImagesAreEqual(UIImage* image_1, UIImage* image_2);

}  // namespace uiimage_utils
}  // namespace test
}  // namespace ui

#endif  // UI_BASE_TEST_IOS_UI_IMAGE_TEST_UTILS_H_

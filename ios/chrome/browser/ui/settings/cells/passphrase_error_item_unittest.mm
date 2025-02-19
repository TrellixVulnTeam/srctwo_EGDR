// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/settings/cells/passphrase_error_item.h"

#include "testing/gtest/include/gtest/gtest.h"
#import "testing/gtest_mac.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace {

// Tests that the text label is set properly after a call to |configureCell:|.
TEST(PasswordDetailsItemTest, ConfigureCell) {
  PassphraseErrorItem* item = [[PassphraseErrorItem alloc] initWithType:0];
  PassphraseErrorCell* cell = [[[item cellClass] alloc] init];
  EXPECT_TRUE([cell isMemberOfClass:[PassphraseErrorCell class]]);
  EXPECT_NSEQ(nil, cell.textLabel.text);
  NSString* text = @"This is an error";

  item.text = text;
  [item configureCell:cell];
  EXPECT_NSEQ(text, cell.textLabel.text);
}

}  // namespace

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/autofill/cells/autofill_edit_item.h"

#include "testing/gtest/include/gtest/gtest.h"
#import "testing/gtest_mac.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace {

// Tests that the label and text field are set properly after a call to
// |configureCell:|.
TEST(AutofillEditItemTest, ConfigureCell) {
  AutofillEditItem* item = [[AutofillEditItem alloc] initWithType:0];
  NSString* name = @"Name";
  NSString* value = @"Value";
  BOOL enabled = NO;

  item.textFieldName = name;
  item.textFieldValue = value;
  item.textFieldEnabled = enabled;

  id cell = [[[item cellClass] alloc] init];
  ASSERT_TRUE([cell isMemberOfClass:[AutofillEditCell class]]);

  AutofillEditCell* autofillEditCell = cell;
  EXPECT_EQ(0U, autofillEditCell.textLabel.text.length);
  EXPECT_EQ(0U, autofillEditCell.textField.text.length);
  EXPECT_TRUE(autofillEditCell.textField.enabled);

  [item configureCell:cell];
  EXPECT_NSEQ(name, autofillEditCell.textLabel.text);
  EXPECT_NSEQ(value, autofillEditCell.textField.text);
  EXPECT_FALSE(autofillEditCell.textField.enabled);
}

}  // namespace

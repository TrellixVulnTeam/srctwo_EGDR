// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/settings/cells/copied_to_chrome_item.h"

#include "components/strings/grit/components_strings.h"
#include "ios/chrome/grit/ios_chromium_strings.h"
#import "ios/third_party/material_components_ios/src/components/Buttons/src/MaterialButtons.h"
#include "testing/gtest/include/gtest/gtest.h"
#import "testing/gtest_mac.h"
#include "ui/base/l10n/l10n_util_mac.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace {

// Tests that the cell created out of a CopiedToChromeItem is set up properly.
TEST(CopiedToChromeItemTest, InitializeCell) {
  CopiedToChromeItem* item = [[CopiedToChromeItem alloc] initWithType:0];

  id cell = [[[item cellClass] alloc] init];
  ASSERT_TRUE([cell isMemberOfClass:[CopiedToChromeCell class]]);

  CopiedToChromeCell* copiedToChromeCell = cell;
  EXPECT_NSEQ(l10n_util::GetNSString(IDS_IOS_AUTOFILL_DESCRIBE_LOCAL_COPY),
              copiedToChromeCell.textLabel.text);
  EXPECT_NSEQ([l10n_util::GetNSString(IDS_AUTOFILL_CLEAR_LOCAL_COPY_BUTTON)
                  uppercaseString],
              [copiedToChromeCell.button titleForState:UIControlStateNormal]);
}

}  // namespace

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/history/history_entry_item.h"

#include "base/i18n/time_formatting.h"
#include "base/strings/sys_string_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "components/history/core/browser/browsing_history_service.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/gtest_mac.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

using history::BrowsingHistoryService;

namespace {
const char kTestUrl[] = "http://test/";
const char kTestUrl2[] = "http://test2/";
const char kTestTitle[] = "Test";
}

HistoryEntryItem* GetHistoryEntryItem(const GURL& url,
                                      const char title[],
                                      base::Time timestamp) {
  BrowsingHistoryService::HistoryEntry entry(
      BrowsingHistoryService::HistoryEntry::LOCAL_ENTRY, GURL(url),
      base::UTF8ToUTF16(title), timestamp, "", false, base::string16(), false);
  HistoryEntryItem* item = [[HistoryEntryItem alloc] initWithType:0
                                                     historyEntry:entry
                                                     browserState:nil
                                                         delegate:nil];
  return item;
}

// Tests that -[HistoryEntryItem configureCell:] sets the cell's textLabel text
// to the item title, the detailTextLabel text to the URL, and the timeLabel
// text to the timestamp.
TEST(HistoryEntryItemTest, ConfigureCell) {
  base::Time timestamp = base::Time::Now();
  HistoryEntryItem* item =
      GetHistoryEntryItem(GURL(kTestUrl), kTestTitle, timestamp);

  HistoryEntryCell* cell = [[[item cellClass] alloc] init];
  EXPECT_TRUE([cell isMemberOfClass:[HistoryEntryCell class]]);
  [item configureCell:cell];
  EXPECT_NSEQ(base::SysUTF8ToNSString(kTestTitle), cell.textLabel.text);
  EXPECT_NSEQ(base::SysUTF8ToNSString(kTestUrl), cell.detailTextLabel.text);
  EXPECT_NSEQ(base::SysUTF16ToNSString(base::TimeFormatTimeOfDay(timestamp)),
              cell.timeLabel.text);
}

// Tests that -[HistoryItem isEqualToHistoryItem:] returns YES if the two items
// have the same URL and timestamp, and NO otherwise.
TEST(HistoryEntryItemTest, IsEqual) {
  base::Time timestamp = base::Time::Now();
  base::Time timestamp2 = timestamp - base::TimeDelta::FromMinutes(1);
  HistoryEntryItem* history_entry =
      GetHistoryEntryItem(GURL(kTestUrl), kTestTitle, timestamp);
  HistoryEntryItem* same_entry =
      GetHistoryEntryItem(GURL(kTestUrl), kTestTitle, timestamp);

  HistoryEntryItem* different_time_entry =
      GetHistoryEntryItem(GURL(kTestUrl), kTestTitle, timestamp2);
  HistoryEntryItem* different_url_entry =
      GetHistoryEntryItem(GURL(kTestUrl2), kTestTitle, timestamp);

  EXPECT_TRUE([history_entry isEqualToHistoryEntryItem:same_entry]);
  EXPECT_FALSE([history_entry isEqualToHistoryEntryItem:different_time_entry]);
  EXPECT_FALSE([history_entry isEqualToHistoryEntryItem:different_url_entry]);
}

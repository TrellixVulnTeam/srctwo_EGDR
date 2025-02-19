// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_READING_LIST_READING_LIST_DATA_SINK_H_
#define IOS_CHROME_BROWSER_UI_READING_LIST_READING_LIST_DATA_SINK_H_

@protocol ReadingListDataSource;

// Data Sink for the Reading List UI, receiving informations from the data
// source.
@protocol ReadingListDataSink

// Called by the data source when it is ready.
- (void)dataSourceReady:(id<ReadingListDataSource>)dataSource;
// Notifies the DataSink that the DataSource has changed and the items should be
// reloaded.
- (void)dataSourceChanged;

// Returns the read items displayed.
- (NSArray<CollectionViewItem*>*)readItems;
// Returns the unread items displayed.
- (NSArray<CollectionViewItem*>*)unreadItems;

// Notifies the DataSink that the |item| has changed and it should be reloaded
// if it is still displayed.
- (void)itemHasChangedAfterDelay:(CollectionViewItem*)item;
// Notifies the DataSink that the |items| have changed and must be reloaded. The
// |items| must be presented.
- (void)itemsHaveChanged:(NSArray<CollectionViewItem*>*)items;

@end

#endif  // IOS_CHROME_BROWSER_UI_READING_LIST_READING_LIST_DATA_SINK_H_

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Globals:
/** @const */ var RESULTS_PER_PAGE = 150;

/**
 * Amount of time between pageviews that we consider a 'break' in browsing,
 * measured in milliseconds.
 * @const
 */
var BROWSING_GAP_TIME = 15 * 60 * 1000;

/**
 * The largest bucket value for UMA histogram, based on entry ID. All entries
 * with IDs greater than this will be included in this bucket.
 * @const
 */
var UMA_MAX_BUCKET_VALUE = 1000;

/**
 * The largest bucket value for a UMA histogram that is a subset of above.
 * @const
 */
var UMA_MAX_SUBSET_BUCKET_VALUE = 100;

/**
 * Histogram buckets for UMA tracking of which view is being shown to the user.
 * Keep this in sync with the HistoryPageView enum in histograms.xml.
 * This enum is append-only.
 * @enum {number}
 */
var HistoryPageViewHistogram = {
  HISTORY: 0,
  DEPRECATED_GROUPED_WEEK: 1,
  DEPRECATED_GROUPED_MONTH: 2,
  SYNCED_TABS: 3,
  SIGNIN_PROMO: 4,
  END: 5,  // Should always be last.
};

/**
 * @const
 */
var SYNCED_TABS_HISTOGRAM_NAME = 'HistoryPage.OtherDevicesMenu';

/**
 * Histogram buckets for UMA tracking of synced tabs.
 * @const
 */
var SyncedTabsHistogram = {
  INITIALIZED: 0,
  SHOW_MENU_DEPRECATED: 1,
  LINK_CLICKED: 2,
  LINK_RIGHT_CLICKED: 3,
  SESSION_NAME_RIGHT_CLICKED_DEPRECATED: 4,
  SHOW_SESSION_MENU: 5,
  COLLAPSE_SESSION: 6,
  EXPAND_SESSION: 7,
  OPEN_ALL: 8,
  HAS_FOREIGN_DATA: 9,
  HIDE_FOR_NOW: 10,
  LIMIT: 11  // Should always be the last one.
};

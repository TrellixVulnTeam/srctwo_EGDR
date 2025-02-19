// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


/** @const */ var Constants = {
  /**
   * Key to access wallpaper rss in chrome.storage.local.
   */
  AccessLocalRssKey: 'wallpaper-picker-surprise-rss-key',

  /**
   * Key to access wallpaper manifest in chrome.storage.local.
   */
  AccessLocalManifestKey: 'wallpaper-picker-manifest-key',

  /**
   * Key to access user wallpaper info in chrome.storage.local.
   */
  AccessLocalWallpaperInfoKey: 'wallpaper-local-info-key',

  /**
   * Key to access user wallpaper info in chrome.storage.sync.
   */
  AccessSyncWallpaperInfoKey: 'wallpaper-sync-info-key',

  /**
   * Key to access last changed date of a surprise wallpaper in
   * chrome.storage.local or chrome.storage.sync.
   */
  AccessLastSurpriseWallpaperChangedDate: 'wallpaper-last-changed-date-key',

  /**
   * Key to access if surprise me feature is enabled or not in
   * chrome.storage.local.
   */
  AccessLocalSurpriseMeEnabledKey: 'surprise-me-enabled-key',

  /**
   * Key to access if surprise me feature is enabled or not in
   * chrome.storage.sync.
   */
  AccessSyncSurpriseMeEnabledKey: 'sync-surprise-me-enabled-key',

  /**
   * Suffix to append to baseURL if requesting high resoultion wallpaper.
   */
  HighResolutionSuffix: '_high_resolution.jpg',

  /**
   * URL to get latest wallpaper RSS feed.
   */
  WallpaperRssURL: 'https://storage.googleapis.com/' +
      'chromeos-wallpaper-public/wallpaper.rss',

  /**
   * cros-wallpaper namespace URI.
   */
  WallpaperNameSpaceURI: 'http://commondatastorage.googleapis.com/' +
      'chromeos-wallpaper-public/cros-wallpaper-uri',

  /**
   * Wallpaper sources enum.
   */
  WallpaperSourceEnum: {
    Online: 'ONLINE',
    Daily: 'DAILY',
    OEM: 'OEM',
    Custom: 'CUSTOM',
    ThirdParty: 'THIRDPARTY',
    AddNew: 'ADDNEW',
    Default: 'DEFAULT'
  },

  /**
   * Local storage.
   */
  WallpaperLocalStorage: chrome.storage.local,

  /**
   * Sync storage.
   */
  WallpaperSyncStorage: chrome.storage.sync,

  /**
   * Suffix to append to file name if it is a thumbnail.
   */
  CustomWallpaperThumbnailSuffix: '_thumbnail',

  /**
   * Wallpaper directory enum.
   */
  WallpaperDirNameEnum: {ORIGINAL: 'original', THUMBNAIL: 'thumbnail'},

  /**
   * The filename prefix for a third party wallpaper.
   */
  ThirdPartyWallpaperPrefix: 'third_party_'
};

// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * A thumbnail loader for list style UI.
 *
 * ListThumbnailLoader is a thubmanil loader designed for list style ui. List
 * thumbnail loader loads thumbnail in a viewport of the UI. ListThumbnailLoader
 * is responsible to return dataUrls of thumbnails and fetch them with proper
 * priority.
 *
 * @param {!DirectoryModel} directoryModel A directory model.
 * @param {!ThumbnailModel} thumbnailModel Thumbnail metadata model.
 * @param {!VolumeManagerWrapper} volumeManager Volume manager.
 * @param {Function=} opt_thumbnailLoaderConstructor A constructor of thumbnail
 *     loader. This argument is used for testing.
 * @struct
 * @constructor
 * @extends {cr.EventTarget}
 */
function ListThumbnailLoader(
    directoryModel, thumbnailModel, volumeManager,
    opt_thumbnailLoaderConstructor) {
  /**
   * @private {!DirectoryModel}
   */
  this.directoryModel_ = directoryModel;

  /**
   * @private {!ThumbnailModel}
   */
  this.thumbnailModel_ = thumbnailModel;

  /**
   * @private {!VolumeManagerWrapper}
   */
  this.volumeManager_ = volumeManager;

  /**
   * Constructor of thumbnail loader.
   * @private {!Function}
   */
  this.thumbnailLoaderConstructor_ =
      opt_thumbnailLoaderConstructor || ThumbnailLoader;

  /**
   * @private {Object<!ListThumbnailLoader.Task>}
   */
  this.active_ = {};

  /**
   * @private {LRUCache<!ListThumbnailLoader.ThumbnailData>}
   */
  this.cache_ = new LRUCache(ListThumbnailLoader.CACHE_SIZE);

  /**
   * @private {number}
   */
  this.beginIndex_ = 0;

  /**
   * @private {number}
   */
  this.endIndex_ = 0;

  /**
   * Cursor.
   * @private {number}
   */
  this.cursor_ = 0;

  /**
   * Current volume type.
   * @private {?ListThumbnailLoader.VolumeType}
   */
  this.currentVolumeType_ = null;

  /**
   * @private {!FileListModel}
   */
  this.dataModel_ = assert(this.directoryModel_.getFileList());

  this.directoryModel_.addEventListener(
      'scan-completed', this.onScanCompleted_.bind(this));
  this.dataModel_.addEventListener('splice', this.onSplice_.bind(this));
  this.dataModel_.addEventListener('sorted', this.onSorted_.bind(this));
  this.dataModel_.addEventListener('change', this.onChange_.bind(this));
}

ListThumbnailLoader.prototype.__proto__ = cr.EventTarget.prototype;

/**
 * Cache size. Cache size must be larger than sum of high priority range size
 * and number of prefetch tasks.
 * @const {number}
 */
ListThumbnailLoader.CACHE_SIZE = 500;

/**
 * Volume type for testing.
 * @const {string}
 */
ListThumbnailLoader.TEST_VOLUME_TYPE = 'test_volume_type';

/**
 * Number of maximum active tasks for testing.
 * @type {number}
 */
ListThumbnailLoader.numOfMaxActiveTasksForTest = 2;

/**
 * @typedef {(VolumeManagerCommon.VolumeType|string)}
 */
ListThumbnailLoader.VolumeType;

/**
 * Gets number of prefetch requests. This number changes based on current volume
 * type.
 * @return {number} Number of prefetch requests.
 * @private
 */
ListThumbnailLoader.prototype.getNumOfPrefetch_ = function () {
  switch (/** @type {?ListThumbnailLoader.VolumeType} */
      (this.currentVolumeType_)) {
    case VolumeManagerCommon.VolumeType.MTP:
      return 0;
    case ListThumbnailLoader.TEST_VOLUME_TYPE:
      return 1;
    default:
      return 20;
  }
};

/**
 * Gets maximum number of active thumbnail fetch tasks. This number changes
 * based on current volume type.
 * @return {number} Maximum number of active thumbnail fetch tasks.
 * @private
 */
ListThumbnailLoader.prototype.getNumOfMaxActiveTasks_ = function() {
  switch (/** @type {?ListThumbnailLoader.VolumeType} */
      (this.currentVolumeType_)) {
    case VolumeManagerCommon.VolumeType.MTP:
      return 1;
    case ListThumbnailLoader.TEST_VOLUME_TYPE:
      return ListThumbnailLoader.numOfMaxActiveTasksForTest;
    default:
      return 10;
  }
};

/**
 * An event handler for scan-completed event of directory model. When directory
 * scan is running, we don't fetch thumbnail in order not to block IO for
 * directory scan. i.e. modification events during directory scan is ignored.
 * We need to check thumbnail loadings after directory scan is completed.
 *
 * @param {!Event} event Event
 */
ListThumbnailLoader.prototype.onScanCompleted_ = function(event) {
  this.cursor_ = this.beginIndex_;
  this.continue_();
};

/**
 * An event handler for splice event of data model. When list is changed, start
 * to rescan items.
 *
 * @param {!Event} event Event
 */
ListThumbnailLoader.prototype.onSplice_ = function(event) {
  this.cursor_ = this.beginIndex_;
  this.continue_();
};

/**
 * An event handler for sorted event of data model. When list is sorted, start
 * to rescan items.
 *
 * @param {!Event} event Event
 */
ListThumbnailLoader.prototype.onSorted_ = function(event) {
  this.cursor_ = this.beginIndex_;
  this.continue_();
};

/**
 * An event handler for change event of data model.
 *
 * @param {!Event} event Event
 */
ListThumbnailLoader.prototype.onChange_ = function(event) {
  // Mark the thumbnail in cache as invalid.
  var entry = this.dataModel_.item(event.index);
  var cachedThumbnail = this.cache_.peek(entry.toURL());
  if (cachedThumbnail)
    cachedThumbnail.outdated = true;

  this.cursor_ = this.beginIndex_;
  this.continue_();
};

/**
 * Sets high priority range in the list.
 *
 * @param {number} beginIndex Begin index of the range, inclusive.
 * @param {number} endIndex End index of the range, exclusive.
 */
ListThumbnailLoader.prototype.setHighPriorityRange = function(
    beginIndex, endIndex) {
  if (!(beginIndex < endIndex))
    return;

  this.beginIndex_ = beginIndex;
  this.endIndex_ = endIndex;
  this.cursor_ = this.beginIndex_;

  this.continue_();
};

/**
 * Returns a thumbnail of an entry if it is in cache. This method returns
 * thumbnail even if the thumbnail is outdated.
 *
 * @return {ListThumbnailLoader.ThumbnailData} If the thumbnail is not in cache,
 *     this returns null.
 */
ListThumbnailLoader.prototype.getThumbnailFromCache = function(entry) {
  // Since we want to evict cache based on high priority range, we use peek here
  // instead of get.
  return this.cache_.peek(entry.toURL()) || null;
};

/**
 * Enqueues tasks if available.
 */
ListThumbnailLoader.prototype.continue_ = function() {
  // If directory scan is running or all items are scanned, do nothing.
  if (this.directoryModel_.isScanning() ||
      !(this.cursor_ < this.dataModel_.length))
    return;

  var entry = /** @type {Entry} */ (this.dataModel_.item(this.cursor_));

  // Check volume type for optimizing the parameters.
  var volumeInfo = this.volumeManager_.getVolumeInfo(entry);
  this.currentVolumeType_ = volumeInfo ? volumeInfo.volumeType : null;

  // If tasks are running full or all items are scanned, do nothing.
  if (!(Object.keys(this.active_).length < this.getNumOfMaxActiveTasks_()) ||
      !(this.cursor_ < this.endIndex_ + this.getNumOfPrefetch_())) {
    return;
  }

  // If the entry is a directory, already in cache as valid or fetching, skip.
  var thumbnail = this.cache_.get(entry.toURL());
  if (entry.isDirectory ||
      (thumbnail && !thumbnail.outdated) ||
      this.active_[entry.toURL()]) {
    this.cursor_++;
    this.continue_();
    return;
  }

  this.enqueue_(this.cursor_, entry);
  this.cursor_++;
  this.continue_();
};

/**
 * Enqueues a thumbnail fetch task for an entry.
 *
 * @param {number} index Index of an entry in current data model.
 * @param {!Entry} entry An entry.
 */
ListThumbnailLoader.prototype.enqueue_ = function(index, entry) {
  var task = new ListThumbnailLoader.Task(
      entry, this.volumeManager_, this.thumbnailModel_,
      this.thumbnailLoaderConstructor_);

  var url = entry.toURL();
  this.active_[url] = task;

  task.fetch().then(function(thumbnail) {
    delete this.active_[url];
    this.cache_.put(url, thumbnail);
    this.dispatchThumbnailLoaded_(index, thumbnail);
    this.continue_();
  }.bind(this));
};

/**
 * Dispatches thumbnail loaded event.
 *
 * @param {number} index Index of an original image in the data model.
 * @param {!ListThumbnailLoader.ThumbnailData} thumbnail Thumbnail.
 */
ListThumbnailLoader.prototype.dispatchThumbnailLoaded_ = function(
    index, thumbnail) {
  // Update index if it's already invalid, i.e. index may be invalid if some
  // change had happened in the data model during thumbnail fetch.
  var item = this.dataModel_.item(index);
  if (item && item.toURL() !== thumbnail.fileUrl) {
    index = -1;;
    for (var i = 0; i < this.dataModel_.length; i++) {
      if (this.dataModel_.item(i).toURL() === thumbnail.fileUrl) {
        index = i;
        break;
      }
    }
  }

  if (index > -1) {
    this.dispatchEvent(
        new ListThumbnailLoader.ThumbnailLoadedEvent(index, thumbnail));
  }
};

/**
 * Thumbnail loaded event.
 * @param {number} index Index of an original image in the current data model.
 * @param {!ListThumbnailLoader.ThumbnailData} thumbnail Thumbnail.
 * @extends {Event}
 * @constructor
 * @struct
 */
ListThumbnailLoader.ThumbnailLoadedEvent = function(index, thumbnail) {
  var event = new Event('thumbnailLoaded');

  /** @type {number} */
  event.index = index;

  /** @type {string}*/
  event.fileUrl = thumbnail.fileUrl;

  /** @type {?string} */
  event.dataUrl = thumbnail.dataUrl;

  /** @type {?number} */
  event.width = thumbnail.width;

  /** @type {?number}*/
  event.height = thumbnail.height;

  return event;
};

/**
 * A class to represent thumbnail data.
 * @param {string} fileUrl File url of an original image.
 * @param {?string} dataUrl Data url of thumbnail.
 * @param {?number} width Width of thumbnail.
 * @param {?number} height Height of thumbnail.
 * @constructor
 * @struct
 */
ListThumbnailLoader.ThumbnailData = function(fileUrl, dataUrl, width, height) {
  /**
   * @const {string}
   */
  this.fileUrl = fileUrl;

  /**
   * @const {?string}
   */
  this.dataUrl = dataUrl;

  /**
   * @const {?number}
   */
  this.width = width;

  /**
   * @const {?number}
   */
  this.height = height;

  /**
   * @type {boolean}
   */
  this.outdated = false;
};

/**
 * A task to load thumbnail.
 *
 * @param {!Entry} entry An entry.
 * @param {!VolumeManagerWrapper} volumeManager Volume manager.
 * @param {!ThumbnailModel} thumbnailModel Metadata cache.
 * @param {!Function} thumbnailLoaderConstructor A constructor of thumbnail
 *     loader.
 * @constructor
 * @struct
 */
ListThumbnailLoader.Task = function(
    entry, volumeManager, thumbnailModel, thumbnailLoaderConstructor) {
  this.entry_ = entry;
  this.volumeManager_ = volumeManager;
  this.thumbnailModel_ = thumbnailModel;
  this.thumbnailLoaderConstructor_ = thumbnailLoaderConstructor;
};

/**
 * Minimum delay of milliseconds before another retry for fetching a thumbnmail
 * from EXIF after failing with an IO error. In milliseconds.
 *
 * @type {number}
 */
ListThumbnailLoader.Task.EXIF_IO_ERROR_DELAY = 3000;

/**
 * Fetches thumbnail.
 *
 * @return {!Promise<!ListThumbnailLoader.ThumbnailData>} A promise which is
 *     resolved when thumbnail data is fetched with either a success or an
 *     error.
 */
ListThumbnailLoader.Task.prototype.fetch = function() {
  var ioError = false;
  return this.thumbnailModel_.get([this.entry_]).then(function(metadatas) {
    // When it failed to read exif header with an IO error, do not generate
    // thumbnail at this time since it may success in the second try. If it
    // failed to read at 0 byte, it would be an IO error.
    if (metadatas[0].thumbnail.urlError &&
        metadatas[0].thumbnail.urlError.errorDescription ===
            'Error: Unexpected EOF @0') {
      ioError = true;
      return Promise.reject();
    }
    return metadatas[0];
  }.bind(this)).then(function(metadata) {
    var loadTargets = [
      ThumbnailLoader.LoadTarget.CONTENT_METADATA,
      ThumbnailLoader.LoadTarget.EXTERNAL_METADATA
    ];

    // If the file is on a provided file system which is based on network, then
    // don't generate thumbnails from file entry, as it could cause very high
    // network traffic.
    var volumeInfo = this.volumeManager_.getVolumeInfo(this.entry_);
    if (volumeInfo && (volumeInfo.volumeType !==
        VolumeManagerCommon.VolumeType.PROVIDED ||
        volumeInfo.source !== VolumeManagerCommon.Source.NETWORK)) {
      loadTargets.push(ThumbnailLoader.LoadTarget.FILE_ENTRY);
    }

    return new this.thumbnailLoaderConstructor_(
        this.entry_, ThumbnailLoader.LoaderType.IMAGE, metadata,
        undefined /* opt_mediaType */, loadTargets)
        .loadAsDataUrl(ThumbnailLoader.FillMode.OVER_FILL);
  }.bind(this)).then(function(result) {
    return new ListThumbnailLoader.ThumbnailData(
        this.entry_.toURL(), result.data, result.width, result.height);
  }.bind(this)).catch(function() {
    // If an error happens during generating of a thumbnail, then return
    // an empty object, so we don't retry the thumbnail over and over
    // again.
    var thumbnailData = new ListThumbnailLoader.ThumbnailData(
          this.entry_.toURL(), null, null, null);
    if (ioError) {
      // If fetching a thumbnail from EXIF fails due to an IO error, then try to
      // refetch it in the future, but not earlier than in 3 second.
      setTimeout(function() {
        thumbnailData.outdated = true;
      }, ListThumbnailLoader.Task.EXIF_IO_ERROR_DELAY);
    }
    return thumbnailData;
  }.bind(this));
};

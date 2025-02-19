// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * Empty folder controller.
 * @param {!EmptyFolder} emptyFolder Empty folder ui.
 * @param {!DirectoryModel} directoryModel Directory model.
 * @constructor
 * @struct
 */
function EmptyFolderController(emptyFolder, directoryModel) {
  /**
   * @private {!EmptyFolder}
   */
  this.emptyFolder_ = emptyFolder;

  /**
   * @private {!DirectoryModel}
   */
  this.directoryModel_ = directoryModel;

  /**
   * @private {!FileListModel}
   */
  this.dataModel_ = assert(this.directoryModel_.getFileList());

  /**
   * @private {boolean}
   */
  this.isScanning_ = false;

  this.directoryModel_.addEventListener(
      'scan-started', this.onScanStarted_.bind(this));
  this.directoryModel_.addEventListener(
      'scan-failed', this.onScanFinished_.bind(this));
  this.directoryModel_.addEventListener(
      'scan-cancelled', this.onScanFinished_.bind(this));
  this.directoryModel_.addEventListener(
      'scan-completed', this.onScanFinished_.bind(this));
  this.directoryModel_.addEventListener(
      'rescan-completed', this.onScanFinished_.bind(this));

  this.dataModel_.addEventListener('splice', this.onSplice_.bind(this));
}

/**
 * Handles splice event.
 * @private
 */
EmptyFolderController.prototype.onSplice_ = function() {
  this.update_();
};

/**
 * Handles scan start.
 * @private
 */
EmptyFolderController.prototype.onScanStarted_ = function() {
  this.isScanning_ = true;
  this.update_();
};

/**
 * Handles scan finish.
 * @private
 */
EmptyFolderController.prototype.onScanFinished_ = function() {
  this.isScanning_ = false;
  this.update_();
};

/**
 * Updates visibility of empty folder UI.
 * @private
 */
EmptyFolderController.prototype.update_ = function() {
  if (!this.isScanning_ && this.dataModel_.length === 0) {
    var query = this.directoryModel_.getLastSearchQuery();
    var html = '';
    if (query) {
      html = strf(
          'SEARCH_NO_MATCHING_FILES_HTML',
          util.htmlEscape(query));
    } else {
      html = str('EMPTY_FOLDER');
    }

    this.emptyFolder_.setMessage(html);
    this.emptyFolder_.show();
  } else {
    this.emptyFolder_.hide();
  }
};

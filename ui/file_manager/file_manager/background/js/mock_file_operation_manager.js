// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

/**
 * Mock class of FileOperationManager.
 * @constructor
 * @extends {cr.EventTarget}
 */
function MockFileOperationManager() {
  cr.EventTarget.call(this);

  /**
   * Event to be dispatched when requestTaskCancel is called.
   * @type {Event}
   */
  this.cancelEvent = null;

  /** @type {!Array<string>} */
  this.generatedTaskIds = [];

  /** @type {Function} */
  this.pasteResolver = null;
}

MockFileOperationManager.prototype = {
  __proto__: cr.EventTarget.prototype
};

/**
 * Dispatches a pre-specified cancel event.
 */
MockFileOperationManager.prototype.requestTaskCancel = function() {
  this.dispatchEvent(this.cancelEvent);
};

/**
 * @param {!Array<!Entry>} sourceEntries Entries of the source files.
 * @param {!DirectoryEntry} targetEntry The destination entry of the target
 *     directory.
 * @param {boolean} isMove True if the operation is "move", otherwise (i.e.
 *     if the operation is "copy") false.
 * @param {string=} opt_taskId If the corresponding item has already created
 *     at another places, we need to specify the ID of the item. If the
 *     item is not created, FileOperationManager generates new ID.
 */
MockFileOperationManager.prototype.paste = function(
    sourceEntries, targetEntry, isMove, opt_taskId) {
  if (this.pasteResolver) {
    this.pasteResolver.call(this, {
      sourceEntries: sourceEntries,
      targetEntry: targetEntry,
      isMove: isMove,
      opt_taskId: opt_taskId
    });
    // Reset the resolver for the next paste call.
    this.pasteResolver = null;
  }
};

/**
 * @return {Promise<Object>} A promise that is resolved the next time #paste is
 * called.  The Object contains the arguments that #paste was called with.
 */
MockFileOperationManager.prototype.whenPasteCalled = function() {
  if (this.pasteResolver) {
    throw new Error('Only one paste call can be waited on at a time.');
  }

  return new Promise(function(resolve, reject) {
    this.pasteResolver = resolve;
  }.bind(this));
};

/**
 * Generates a unique task Id.
 * @return {string}
 */
MockFileOperationManager.prototype.generateTaskId = function() {
  var newTaskId = 'task' + this.generatedTaskIds.length;
  this.generatedTaskIds.push(newTaskId);
  return newTaskId;
};

/**
 * @return {boolean} Whether or not the given task ID belongs to a task
 *     generated by this manager.
 */
MockFileOperationManager.prototype.isKnownTaskId = function(id) {
  return this.generatedTaskIds.indexOf(id) !== -1;
};

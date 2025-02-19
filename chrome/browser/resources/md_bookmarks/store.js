// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview A singleton datastore for the Bookmarks page. Page state is
 * publicly readable, but can only be modified by dispatching an Action to
 * the store.
 */

cr.define('bookmarks', function() {
  /** @constructor */
  function Store() {
    /** @type {!BookmarksPageState} */
    this.data_ = bookmarks.util.createEmptyState();
    /** @type {boolean} */
    this.initialized_ = false;
    /** @type {!Array<DeferredAction>} */
    this.queuedActions_ = [];
    /** @type {!Array<!StoreObserver>} */
    this.observers_ = [];
    /** @private {boolean} */
    this.batchMode_ = false;
  }

  Store.prototype = {
    /**
     * @param {!BookmarksPageState} initialState
     */
    init: function(initialState) {
      this.data_ = initialState;

      this.queuedActions_.forEach((action) => {
        this.dispatchInternal_(action);
      });

      this.initialized_ = true;
      this.notifyObservers_(this.data_);
    },

    /** @type {!BookmarksPageState} */
    get data() {
      return this.data_;
    },

    /** @return {boolean} */
    isInitialized: function() {
      return this.initialized_;
    },

    /** @param {!StoreObserver} observer */
    addObserver: function(observer) {
      this.observers_.push(observer);
    },

    /** @param {!StoreObserver} observer */
    removeObserver: function(observer) {
      var index = this.observers_.indexOf(observer);
      this.observers_.splice(index, 1);
    },

    /**
     * Begin a batch update to store data, which will disable updates to the
     * UI until `endBatchUpdate` is called. This is useful when a single UI
     * operation is likely to cause many sequential model updates (eg, deleting
     * 100 bookmarks).
     */
    beginBatchUpdate: function() {
      this.batchMode_ = true;
    },

    /**
     * End a batch update to the store data, notifying the UI of any changes
     * which occurred while batch mode was enabled.
     */
    endBatchUpdate: function() {
      this.batchMode_ = false;
      this.notifyObservers_(this.data);
    },

    /**
     * Handles a 'deferred' action, which can asynchronously dispatch actions
     * to the Store in order to reach a new UI state. DeferredActions have the
     * form `dispatchAsync(function(dispatch) { ... })`). Inside that function,
     * the |dispatch| callback can be called asynchronously to dispatch Actions
     * directly to the Store.
     * @param {DeferredAction} action
     */
    dispatchAsync: function(action) {
      if (!this.initialized_) {
        this.queuedActions_.push(action);
        return;
      }

      this.dispatchInternal_(action);
    },

    /**
     * Transition to a new UI state based on the supplied |action|, and notify
     * observers of the change. If the Store has not yet been initialized, the
     * action will be queued and performed upon initialization.
     * @param {?Action} action
     */
    dispatch: function(action) {
      this.dispatchAsync(function(dispatch) {
        dispatch(action);
      });
    },

    /**
     * @param {DeferredAction} action
     */
    dispatchInternal_: function(action) {
      action(this.reduce_.bind(this));
    },

    /**
     * @param {?Action} action
     * @private
     */
    reduce_: function(action) {
      if (!action)
        return;

      this.data_ = bookmarks.reduceAction(this.data_, action);
      // Batch notifications until after all initialization queuedActions are
      // resolved.
      if (this.isInitialized() && !this.batchMode_)
        this.notifyObservers_(this.data_);
    },

    /**
     * @param {!BookmarksPageState} state
     * @private
     */
    notifyObservers_: function(state) {
      this.observers_.forEach(function(o) {
        o.onStateChanged(state);
      });
    },
  };

  cr.addSingletonGetter(Store);

  return {
    Store: Store,
  };
});

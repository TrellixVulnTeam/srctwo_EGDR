// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * Fakes a Chrome event that supports one listener.
 * @constructor
 * @extends {ChromeEvent}
 */
function FakeChromeEvent() {
  /**
   * @private
   * @type {Function}
   */
  this.listener_ = null;
}

FakeChromeEvent.prototype = {
  /**
   * Fakes the corresponding call on a Chrome event.  Sets the listener and
   * fails the test if it is already set.
   * @param {Function} listener The new listener.
   */
  addListener: function(listener) {
    this.assertNoListener();
    this.listener_ = listener;
  },

  /**
   * Gets the listener of the event, failing the test if there's none.
   * @return {Function} The event's listener.
   */
  getListener: function() {
    assertNotEquals(null, this.listener_);
    return this.listener_;
  },

  /**
   * Asserts that this object doesn't have any listener added.
   */
  assertNoListener: function() {
    assertEquals(null, this.listener_);
  }
};

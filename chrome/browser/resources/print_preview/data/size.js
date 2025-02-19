// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

cr.define('print_preview', function() {
  'use strict';

  /**
   * Immutable two-dimensional size.
   * @param {number} width Width of the size.
   * @param {number} height Height of the size.
   * @constructor
   */
  function Size(width, height) {
    /**
     * Width of the size.
     * @type {number}
     * @private
     */
    this.width_ = width;

    /**
     * Height of the size.
     * @type {number}
     * @private
     */
    this.height_ = height;
  }

  Size.prototype = {
    /** @return {number} Width of the size. */
    get width() {
      return this.width_;
    },

    /** @return {number} Height of the size. */
    get height() {
      return this.height_;
    },

    /**
     * @param {print_preview.Size} other Other size object to compare against.
     * @return {boolean} Whether this size object is equal to another.
     */
    equals: function(other) {
      return other != null && this.width_ == other.width_ &&
          this.height_ == other.height_;
    }
  };

  // Export
  return {Size: Size};
});

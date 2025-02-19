// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview PasswordListItem represents one row in the list of passwords.
 * It needs to be its own component because FocusRowBehavior provides good a11y.
 */

Polymer({
  is: 'password-list-item',

  behaviors: [FocusRowBehavior, ShowPasswordBehavior],

  /**
   * Selects the password on tap if revealed.
   * @private
   */
  onReadonlyInputTap_: function() {
    if (this.password)
      this.$$('#password').select();
  },

  /**
   * Opens the password action menu.
   * @private
   */
  onPasswordMenuTap_: function() {
    this.fire(
        'password-menu-tap', {target: this.$.passwordMenu, item: this.item});
  },
});

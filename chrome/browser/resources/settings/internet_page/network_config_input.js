// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Polymer element for network configuration input fields.
 */
Polymer({
  is: 'network-config-input',

  properties: {
    label: String,

    disabled: Boolean,

    value: {
      type: String,
      notify: true,
    }
  },
});

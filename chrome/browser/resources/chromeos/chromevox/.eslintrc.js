// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

module.exports = {
  'env': {
    'browser': true,
    'es6': true,
  },
  'rules': {
    // Override restrictions for document.getElementById usage since,
    // chrome://resources/js/util.js is not accessible for chromevox.
    'no-restricted-properties': 'off',
  },
};

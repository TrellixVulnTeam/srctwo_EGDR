// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

chrome.test.runTests([
  function testCreateWithActiveTrue() {
    chrome.test.listenOnce(chrome.tabs.onCreated,
      function(tab) {
        chrome.test.assertEq(tab.active, true);
      });
    chrome.tabs.create({url: 'chrome://newtab/', active: true});
  },
  function testCreateWithActiveFalse() {
    chrome.test.listenOnce(chrome.tabs.onCreated,
      function(tab) {
        chrome.test.assertEq(tab.active, false);
      });
    chrome.tabs.create({url: 'chrome://newtab/', active: false});
  }
]);

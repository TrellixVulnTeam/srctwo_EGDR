// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

chrome.test.runTests([
  function getSelectedPopup() {
    chrome.windows.create(
        {type: 'popup', url: 'about:blank', focused: true},
        chrome.test.callbackPass(function(win) {
      chrome.tabs.query(
          {active: true, windowId: chrome.windows.WINDOW_ID_CURRENT},
          chrome.test.callbackPass(function(tabs) {
        chrome.test.assertEq(1, tabs.length);
        chrome.test.assertTrue(tabs[0].windowId == win.id);
        chrome.test.assertFalse(tabs[0].id == chrome.tabs.TAB_ID_NONE);
      }));
    }));
  }
]);

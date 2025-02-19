// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

chrome.test.runTests([
  function getDisplayPath() {
    chrome.fileSystem.chooseEntry(chrome.test.callbackPass(function(entry) {
      chrome.test.assertEq('gold.txt', entry.name);

      // Test that we can get the display path of the file, and that it begins
      // with the home directory indicator (currently "~").
      chrome.fileSystem.getDisplayPath(entry, chrome.test.callbackPass(
          function(path) {
        chrome.test.assertTrue(path.indexOf("~/") == 0 ||  // POSIX
                               path.indexOf("~\\") == 0);  // Windows
        chrome.test.assertTrue(path.indexOf("gold.txt") >= 0);
      }));
    }));
  }
]);

// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var tests = [
  /**
   * Test that the correct title is displayed for test-whitespace-title.pdf.
   */
  function testHasCorrectTitle() {
    chrome.test.assertEq('test-whitespace-title.pdf', document.title);

    chrome.test.succeed();
  }
];

var scriptingAPI = new PDFScriptingAPI(window, window);
scriptingAPI.setLoadCallback(function() {
  chrome.test.runTests(tests);
});

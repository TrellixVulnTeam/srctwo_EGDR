// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function testExecuteScriptInNewTab() {
  // Create a new tab to chrome://newtab and wait for the loading to complete.
  // Then, try to inject a script into that tab. The injection should fail.
  chrome.tabs.onUpdated.addListener(function listener(tabId, changeInfo, tab) {
    if (tab.url != 'chrome://newtab/' || changeInfo.status != 'complete')
      return;
    chrome.tabs.onUpdated.removeListener(listener);
    chrome.tabs.executeScript(tab.id, {file: 'script.js'}, function() {
      chrome.test.assertTrue(!!chrome.runtime.lastError);
      chrome.test.assertTrue(
          chrome.runtime.lastError.message.indexOf(
              'Cannot access contents of') != -1,
          chrome.runtime.lastError.message);
      chrome.test.succeed();
    });
  });
  chrome.tabs.create({url: 'chrome://newtab'});
}

chrome.test.sendMessage('ready', function() {
  chrome.test.runTests([testExecuteScriptInNewTab]);
});

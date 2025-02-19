// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Called when the user clicks on the browser action.
chrome.browserAction.onClicked.addListener(function(windowId) {
  chrome.test.sendMessage('basics browser action');
});

// Called when the user activates the command.
chrome.commands.onCommand.addListener(function(command) {
  chrome.test.sendMessage(command);
});

chrome.test.notifyPass();

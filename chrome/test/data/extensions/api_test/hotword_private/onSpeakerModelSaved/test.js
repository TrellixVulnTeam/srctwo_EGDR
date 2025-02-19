// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

chrome.test.runTests([
  function onSpeakerModelSaved() {
    chrome.hotwordPrivate.onSpeakerModelSaved.addListener(function () {
      chrome.test.sendMessage("notification");
      chrome.test.succeed();
    });
    chrome.hotwordPrivate.notifySpeakerModelSaved(
        chrome.test.callbackPass(function() {}));
    chrome.test.sendMessage("ready");
  }
]);

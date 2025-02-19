// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var message = {
  messageId: "message-id",
  destinationId: "destination-id",
  timeToLive: 100,
  data: {
    "key1": "value1",
    "key2": "value2"
  }
};

chrome.test.runTests([
  function testSend() {
    chrome.gcm.send(message, function(messageId) {
      chrome.test.succeed();
    });
  }
]);

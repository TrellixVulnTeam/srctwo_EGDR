// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

chrome.app.runtime.onLaunched.addListener(function() {
  chrome.app.window.create('empty.html', {
    hidden: true,
  }, function (win) {
    chrome.test.sendMessage('Launched', function () {
      setTimeout(function () {
        win.show();
        chrome.test.sendMessage('Shown');
      }, 3000);
    });
  });
});

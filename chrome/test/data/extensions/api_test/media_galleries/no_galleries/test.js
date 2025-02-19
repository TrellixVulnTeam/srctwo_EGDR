// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var mediaGalleries = chrome.mediaGalleries;

var mediaFileSystemsListCallback = function(results) {
  chrome.test.assertEq(0, results.length);
};

chrome.test.runTests([
  function mediaGalleriesNoGalleries() {
    mediaGalleries.getMediaFileSystems(
        chrome.test.callbackPass(mediaFileSystemsListCallback));
  },
]);

// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

window.onload = function() {
  var download = document.getElementById('download');
  download.onclick = function() {
    chrome.runtime.sendMessage('icons');
    download.disabled = true;
    return false;
  };
};

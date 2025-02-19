// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function createMainWindow() {
  chrome.app.window.create("window.html", {
    'bounds': {
      'width' : 800,
      'height' : 800
    }
  });
}

chrome.app.runtime.onLaunched.addListener(createMainWindow);

// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

chrome.runtime.onMessageExternal.addListener(
  function(request, sender, sendResponse) {
    if (request.ping)
      sendResponse({pingResponse: 'true'});
});

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

chrome.test.sendMessage('Launched');
chrome.permissions.request({permissions: ['serial']}, function(result) {
  chrome.test.sendMessage('PermissionRequestDone');
});

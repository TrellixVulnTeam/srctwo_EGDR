// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

chrome.test.runTests([

  function nodefineFunc() {
    chrome.test.assertEq("undefined", typeof(chrome.idltest.nodefineFunc));
    chrome.test.succeed();
  }

]);

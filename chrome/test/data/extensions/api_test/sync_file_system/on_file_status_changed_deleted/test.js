// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function setupListener() {
  chrome.syncFileSystem.onFileStatusChanged.addListener(fileInfoReceived);
  chrome.syncFileSystem.requestFileSystem(function() {});
}

// Confirm contents of FileEntry, should still be valid for deleted file.
function fileInfoReceived(fileInfo) {
  // FileEntry object fields.
  var fileEntry = fileInfo.fileEntry;
  chrome.test.assertEq("foo.txt", fileEntry.name);
  chrome.test.assertEq("/foo.txt", fileEntry.fullPath);
  chrome.test.assertTrue(fileEntry.isFile);
  chrome.test.assertFalse(fileEntry.isDirectory);

  chrome.test.assertEq("synced", fileInfo.status);
  chrome.test.assertEq("deleted", fileInfo.action);
  chrome.test.assertEq("remote_to_local", fileInfo.direction);

  // Try to open file using FileEntry, should fail cause file was deleted.
  fileEntry.file(getFileObjectSucceeded, getFileObjectFailed);
}

function getFileObjectFailed(e) {
  chrome.test.assertEq('NotFoundError', e.name);
  chrome.test.succeed();
}

function getFileObjectSucceeded(file_object) {
  chrome.test.fail("Synchronized file deleted, FileEntry.file() should fail.");
}

chrome.test.runTests([
  setupListener
]);

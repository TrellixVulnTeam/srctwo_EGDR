// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function truncateFailByQuota(fs) {
  fs.root.getFile('fd', {create: false, exclusive: false}, function(fileEntry) {
    fileEntry.createWriter(function(fileWriter) {
      var failedInTruncate = false;
      fileWriter.onerror = function(e) {
        failedInTruncate = true;
      };
      fileWriter.onwriteend = function(e) {
        if (failedInTruncate) {
          fail(e.currentTarget.error);
        } else {
          done();
        }
      };
      fileWriter.truncate(2500 * 1024);
    }, unexpectedErrorCallback)
  }, function(e) { fail('Open for 2nd truncate:' + fileErrorToString(e)); } );
}

function requestFileSystemSuccess(fs) {
  fs.root.getFile('fd', {create: true, exclusive: false}, function(fileEntry) {
    fileEntry.createWriter(function(fileWriter) {
      var failedInTruncate = false;
      fileWriter.onerror = function(e) {
        debug(e.currentTarget.error);
        failedInTruncate = true;
      };
      fileWriter.onwriteend = function() {
        if (failedInTruncate) {
          truncateFailByQuota(fs);
        } else {
          fail('Unexpectedly succeeded to truncate.  It should fail by quota.');
        }
      };
      fileWriter.truncate(10000 * 1024);
    }, unexpectedErrorCallback)
  }, function(e) { fail('Open for 1st truncate:' + fileErrorToString(e)); } );
}

function quotaSuccess(usage, quota) {
  if (usage != 0)
    fail('Usage is not zero: ' + usage);
  if (quota != 5000 * 1024)
    fail('Quota is not 5000KiB: ' + quota);

  window.webkitRequestFileSystem(
      window.TEMPORARY,
      1024 * 1024,
      requestFileSystemSuccess,
      unexpectedErrorCallback);
}

function test() {
  if (window.webkitStorageInfo) {
    debug('Querying usage and quota.');
    webkitStorageInfo.queryUsageAndQuota(webkitStorageInfo.TEMPORARY,
                                         quotaSuccess,
                                         unexpectedErrorCallback);
  } else {
    debug('This test requires window.webkitStorageInfo.');
  }
}
